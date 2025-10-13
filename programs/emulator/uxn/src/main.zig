// SPDX-FileCopyrightText: 2025 iyzsong@envs.net
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");
const kos = @import("kolibri.zig");
const uxn = @import("uxn-core");
const varvara = @import("uxn-varvara");

const allocator = kos.allocator;
export var _cmdline: [1024]u8 = undefined;

pub const std_options: std.Options = .{
    .log_level = .info,
    .logFn = struct {
        fn log(comptime level: std.log.Level, comptime scope: @Type(.enum_literal), comptime format: []const u8, args: anytype) void {
            _ = level;
            _ = scope;
            kos.debug_writer.print(format, args) catch return;
        }
    }.log,
};

const VarvaraDefault = varvara.VarvaraSystem(kos.DebugWriter, kos.DebugWriter);
const emu = struct {
    var cpu: uxn.Cpu = undefined;
    var sys: VarvaraDefault = undefined;
    var rom: *[0x10000]u8 = undefined;
    var pixels: []u8 = undefined;
    var screen_width: u32 = undefined;
    var screen_height: u32 = undefined;
    var null_cursor: u32 = undefined;
    var hide_cursor: bool = false;
    var audio_thread: ?u32 = null;
    var scale: u4 = 1;

    fn init(rompath: [*:0]const u8) !void {
        const screen = &emu.sys.screen_device;
        var rom_file = kos.File{ .pathname = rompath };
        emu.rom = try uxn.loadRom(allocator, rom_file.reader());
        emu.cpu = uxn.Cpu.init(emu.rom);
        emu.sys = try VarvaraDefault.init(allocator, kos.debug_writer, kos.debug_writer);
        emu.cpu.device_intercept = struct {
            var file_offsets: [2]u64 = .{ 0, 0 };

            fn bcd8(x: u8) u8 {
                return (x & 0xf) + 10 * ((x & 0xf0) >> 4);
            }

            fn getFilePortSlice(dev: *varvara.file.File, comptime port: comptime_int) []u8 {
                const ports = varvara.file.ports;
                const ptr: usize = dev.loadPort(u16, &cpu, port);

                return if (port == ports.name)
                    std.mem.sliceTo(cpu.mem[ptr..], 0x00)
                else
                    return cpu.mem[ptr..ptr +| dev.loadPort(u16, &cpu, ports.length)];
            }

            pub fn intercept(self: *uxn.Cpu, addr: u8, kind: uxn.Cpu.InterceptKind, data: ?*anyopaque) !void {
                _ = data;
                const port: u4 = @truncate(addr & 0x0f);
                if (audio_thread == null and addr >= 0x30 and addr < 0x70) {
                    audio_thread = kos.createThread(&audio, allocator.alloc(u8, 32 * 1024) catch unreachable);
                }
                switch (addr >> 4) {
                    0xa, 0xb => {
                        if (kind != .output)
                            return;

                        const idx = (addr >> 4) - 0xa;
                        const dev = &sys.file_devices[idx];
                        const ports = varvara.file.ports;
                        switch (port) {
                            ports.stat + 1 => {
                                // TODO: file/directory stat
                                dev.storePort(u16, &cpu, ports.success, 0);
                            },
                            ports.delete => {
                                const name_slice = getFilePortSlice(dev, ports.name);
                                _ = kos.deleteFile(@ptrCast(name_slice)) catch {};
                                dev.storePort(u16, &cpu, ports.success, 0);
                            },
                            ports.name + 1 => {
                                file_offsets[idx] = 0;
                                dev.storePort(u16, &cpu, ports.success, 1);
                            },
                            ports.read + 1 => {
                                const name_slice = getFilePortSlice(dev, ports.name);
                                const data_slice = getFilePortSlice(dev, ports.read);
                                const ret: u32 = kos.readFile(@ptrCast(name_slice), file_offsets[idx], data_slice) catch 0;
                                file_offsets[idx] += ret;
                                dev.storePort(u16, &cpu, ports.success, @intCast(ret));
                            },
                            ports.write + 1 => {
                                const append = dev.loadPort(u8, &cpu, ports.append) == 0x01;
                                const pathname: [*:0]const u8 = @ptrCast(getFilePortSlice(dev, ports.name));
                                const buffer = getFilePortSlice(dev, ports.write);
                                var ret: u32 = 0;
                                if (append) {
                                    const offset: u32 = @intCast(kos.fileGetSize(pathname) catch 0);
                                    ret = kos.writeFile(pathname, offset, buffer) catch |err| blk: {
                                        if (err == error.FileNotFound) {
                                            break :blk kos.createFile(pathname, buffer) catch 0;
                                        } else {
                                            break :blk 0;
                                        }
                                    };
                                } else {
                                    ret = kos.createFile(pathname, buffer) catch 0;
                                }
                                dev.storePort(u16, &cpu, ports.success, @intCast(ret));
                            },
                            else => {},
                        }
                    },
                    0xc => {
                        if (kind != .input)
                            return;

                        const dev = &sys.datetime_device;
                        const date = kos.getSysDate();
                        const time = kos.getSysTime();
                        switch (port) {
                            0x0, 0x1 => {
                                const year: u8 = bcd8(@truncate(date & 0xff));
                                dev.storePort(u16, &cpu, 0x0, @as(u16, 2000) + bcd8(year));
                            },
                            0x02 => {
                                const month: u8 = bcd8(@truncate((date & 0xff00) >> 8));
                                dev.storePort(u8, &cpu, port, month);
                            },
                            0x03 => {
                                const day: u8 = bcd8(@truncate((date & 0xff0000) >> 16));
                                dev.storePort(u8, &cpu, port, day);
                            },
                            0x04 => {
                                const hour: u8 = bcd8(@truncate(time & 0xff));
                                dev.storePort(u8, &cpu, port, hour);
                            },
                            0x05 => {
                                const minute: u8 = bcd8(@truncate((time & 0xff00) >> 8));
                                dev.storePort(u8, &cpu, port, minute);
                            },
                            0x06 => {
                                const second: u8 = bcd8(@truncate((time & 0xff0000) >> 16));
                                dev.storePort(u8, &cpu, port, second);
                            },
                            else => {},
                        }
                    },
                    else => try emu.sys.intercept(self, addr, kind),
                }
            }
        }.intercept;
        emu.cpu.output_intercepts = varvara.full_intercepts.output;
        emu.cpu.input_intercepts = varvara.full_intercepts.input;

        try emu.cpu.evaluateVector(0x0100);
        screen_width = screen.width * emu.scale;
        screen_height = screen.height * emu.scale;
        emu.pixels = try allocator.alloc(u8, @as(usize, 4) * screen_width * screen_height);
    }

    fn exit() void {
        if (audio_thread) |tid| {
            kos.terminateThreadId(tid);
        }
        kos.terminateProcess();
    }

    fn audio() void {
        var samples: [8192]i16 = undefined;
        var sig: kos.Signal = undefined;
        const buf = kos.Sound.init().createBuffer(3 | 0x10000000, 0);
        buf.play(0);
        while (true) {
            kos.waitSignal(&sig);
            if (sig.kind != 0xFF000001) continue;
            @memset(&samples, 0);
            for (0..samples.len / 512) |i| {
                const w = samples[i * 512 .. (i + 1) * 512];
                for (&sys.audio_devices) |*poly| {
                    if (poly.duration <= 0) {
                        poly.evaluateFinishVector(&cpu) catch unreachable;
                    }
                    poly.updateDuration();
                    poly.renderAudio(w);
                }
            }
            for (0..samples.len) |i| {
                samples[i] <<= 6;
            }
            buf.set(@ptrCast(&samples), sig.data2);
        }
    }

    fn update() !void {
        const screen = &sys.screen_device;
        const colors = &sys.system_device.colors;
        if (sys.system_device.exit_code) |_| {
            exit();
        }
        if (screen_width != screen.width * scale or screen_height != screen.height * scale) {
            const skin_height = kos.getSkinHeight();
            allocator.free(emu.pixels);
            screen_width = screen.width * scale;
            screen_height = screen.height * scale;
            emu.pixels = try allocator.alloc(u8, @as(usize, 4) * screen_width * screen_height);
            kos.changeWindow(100, 100, screen_width + 9, screen_height + skin_height + 4);
        }
        try screen.evaluateFrame(&cpu);
        if (screen.dirty_region) |region| {
            for (region.y0..region.y1) |y| {
                for (region.x0..region.x1) |x| {
                    const idx = y * screen.width + x;
                    const pal = (@as(u4, screen.foreground[idx]) << 2) | screen.background[idx];
                    const color = colors[if ((pal >> 2) > 0) (pal >> 2) else (pal & 0x3)];
                    for (0..scale) |sy| {
                        for (0..scale) |sx| {
                            pixels[4 * ((y * scale + sy) * screen.width * scale + (x * scale + sx)) + 2] = color.r;
                            pixels[4 * ((y * scale + sy) * screen.width * scale + (x * scale + sx)) + 1] = color.g;
                            pixels[4 * ((y * scale + sy) * screen.width * scale + (x * scale + sx)) + 0] = color.b;
                        }
                    }
                }
            }
            const ix = region.x0 * scale;
            const iy = region.y0 * scale;
            const iw = (region.x1 - region.x0) * scale;
            const ih = (region.y1 - region.y0) * scale;
            kos.blitter(ix, iy, iw, ih, ix, iy, iw, ih, @ptrCast(emu.pixels.ptr), screen.width * scale * 4, .{});
            screen.dirty_region = null;
        }
    }

    fn changeScale() void {
        const screen = &sys.screen_device;
        scale = switch (scale) {
            1 => 2,
            2 => 3,
            3 => 1,
            else => 1,
        };
        screen.forceRedraw();
    }
};

export fn _start() noreturn {
    const cursor: [32 * 32]u32 = .{0} ** (32 * 32);
    var counter: u32 = 0;
    var last_tick = kos.systemGetTimeCount();

    _ = kos.heapInit();
    _ = kos.setEventsMask(.{ .mouse = true });
    kos.setInputMode(.scancodes);
    emu.null_cursor = kos.loadCursorIndirect(&cursor, 0, 0);
    emu.init(@ptrCast(&_cmdline)) catch unreachable;

    const screen = &emu.sys.screen_device;
    const controller = &emu.sys.controller_device;

    const callbacks = struct {
        fn redraw() void {
            const skin_height = kos.getSkinHeight();
            kos.beginDraw();
            kos.createWindow(300, 300, screen.width * emu.scale + 9, screen.height * emu.scale + skin_height + 4, 0x000000, .{ .skinned = true, .no_fill = true, .relative_coordinates = true }, "UXN");
            kos.endDraw();
        }

        fn key() void {
            const symtab: [0x80]u8 = .{
                // 0x0*
                0,   27,  '1', '2', '3', '4', '5', '6', '7',  '8', '9', '0',  '-',  '=', 8,   '\t',
                // 0x1*
                'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',  'p', '[', ']',  '\r', 0,   'a', 's',
                // 0x2*
                'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z',  'x', 'c', 'v',
                // 0x3*
                'b', 'n', 'm', ',', '.', '/', 0,   0,   0,    ' ', 0,   0,    0,    0,   0,   0,
                // 0x00* + SHIFT
                0,   27,  '!', '@', '#', '$', '%', '^', '&',  '*', '(', ')',  '_',  '+', 8,   '\t',
                // 0x10* + SHIFT
                'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O',  'P', '{', '}',  '\r', 0,   'A', 'S',
                // 0x20* + SHIFT
                'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"',  '~', 0,   '|',  'Z',  'X', 'C', 'V',
                // 0x30* + SHIFT,
                'B', 'N', 'M', '<', '>', '?', 0,   0,   0,    ' ', 0,   0,    0,    0,   0,   0,
            };
            const scancode1 = kos.getKey().key;
            const input: ?union(enum) {
                button: struct {
                    flags: varvara.controller.ButtonFlags,
                    pressed: bool,
                },
                key: u8,
            } = switch (scancode1) {
                0xe0 => blk: {
                    const scancode2 = kos.getKey().key;
                    break :blk switch (scancode2) {
                        0x1d => .{ .button = .{ .flags = .{ .ctrl = true }, .pressed = true } },
                        0x9d => .{ .button = .{ .flags = .{ .ctrl = true }, .pressed = false } },
                        0x38 => .{ .button = .{ .flags = .{ .alt = true }, .pressed = true } },
                        0xb8 => .{ .button = .{ .flags = .{ .alt = true }, .pressed = false } },
                        0x47 => .{ .button = .{ .flags = .{ .start = true }, .pressed = true } },
                        0xc7 => .{ .button = .{ .flags = .{ .start = true }, .pressed = false } },
                        0x48 => .{ .button = .{ .flags = .{ .up = true }, .pressed = true } },
                        0xc8 => .{ .button = .{ .flags = .{ .up = true }, .pressed = false } },
                        0x50 => .{ .button = .{ .flags = .{ .down = true }, .pressed = true } },
                        0xd0 => .{ .button = .{ .flags = .{ .down = true }, .pressed = false } },
                        0x4b => .{ .button = .{ .flags = .{ .left = true }, .pressed = true } },
                        0xcb => .{ .button = .{ .flags = .{ .left = true }, .pressed = false } },
                        0x4d => .{ .button = .{ .flags = .{ .right = true }, .pressed = true } },
                        0xcd => .{ .button = .{ .flags = .{ .right = true }, .pressed = false } },
                        else => null,
                    };
                },
                0x3b => blk: {
                    emu.changeScale();
                    break :blk null;
                },
                0x1d => .{ .button = .{ .flags = .{ .ctrl = true }, .pressed = true } },
                0x9d => .{ .button = .{ .flags = .{ .ctrl = true }, .pressed = false } },
                0x38 => .{ .button = .{ .flags = .{ .alt = true }, .pressed = true } },
                0xb8 => .{ .button = .{ .flags = .{ .alt = true }, .pressed = false } },
                0x2a, 0x36 => .{ .button = .{ .flags = .{ .shift = true }, .pressed = true } },
                0xaa, 0xb6 => .{ .button = .{ .flags = .{ .shift = true }, .pressed = false } },
                else => blk: {
                    if (scancode1 > 0x40) {
                        break :blk null;
                    }
                    const ctrls = kos.getControlKeys();
                    const k = if (ctrls.left_shift or ctrls.right_shift) symtab[scancode1 + 0x40] else symtab[scancode1];
                    break :blk if (k > 0) .{ .key = k } else null;
                },
            };

            if (input) |v| {
                switch (v) {
                    .button => {
                        if (v.button.pressed) {
                            controller.pressButtons(&emu.cpu, v.button.flags, 0) catch unreachable;
                        } else {
                            controller.releaseButtons(&emu.cpu, v.button.flags, 0) catch unreachable;
                        }
                    },
                    .key => {
                        controller.pressKey(&emu.cpu, v.key) catch unreachable;
                    },
                }
            }
        }

        fn button() void {
            const btn = kos.getButton();
            if (btn >> 8 == 1)
                emu.exit();
        }

        fn mouse() void {
            const dev = &emu.sys.mouse_device;
            const pos = kos.mouseGetWindowPosition();
            const events = kos.mouseGetEvents();
            const mouse_pressed: varvara.mouse.ButtonFlags = .{
                .left = events.left_pressed,
                .middle = events.middle_pressed,
                .right = events.right_pressed,
                ._unused = 0,
            };
            const mouse_released: varvara.mouse.ButtonFlags = .{
                .left = events.left_released,
                .middle = events.middle_released,
                .right = events.right_released,
                ._unused = 0,
            };
            if (emu.hide_cursor and pos.y > emu.screen_height) {
                _ = kos.setCursor(0);
                emu.hide_cursor = false;
            }
            if (!emu.hide_cursor and pos.y < emu.screen_height) {
                _ = kos.setCursor(emu.null_cursor);
                emu.hide_cursor = true;
            }
            dev.updatePosition(&emu.cpu, pos.x / emu.scale, pos.y / emu.scale) catch unreachable;
            if (@as(u8, @bitCast(mouse_pressed)) > 0) {
                dev.pressButtons(&emu.cpu, mouse_pressed) catch unreachable;
            }
            if (@as(u8, @bitCast(mouse_released)) > 0) {
                dev.releaseButtons(&emu.cpu, mouse_released) catch unreachable;
            }
        }
    };

    callbacks.redraw();
    while (true) {
        while (true) {
            const event = kos.checkEvent();
            switch (event) {
                .none => break,
                .redraw => callbacks.redraw(),
                .key => callbacks.key(),
                .button => callbacks.button(),
                .mouse => callbacks.mouse(),
                else => {},
            }
        }
        const tick = kos.systemGetTimeCount();
        counter += (tick - last_tick) * 3;
        last_tick = tick;

        if (counter > 5) {
            counter -= 5;
            emu.update() catch unreachable;
        } else {
            kos.sleep(1);
        }
    }
}
