// SPDX-FileCopyrightText: 2025 iyzsong@envs.net
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

pub const SYS = enum(i32) {
    terminate_process = -1,
    create_window = 0,
    put_pixel = 1,
    get_key = 2,
    get_sys_time = 3,
    draw_text = 4,
    sleep = 5,
    put_image = 7,
    define_button = 8,
    thread_info = 9,
    wait_event = 10,
    check_event = 11,
    redraw = 12,
    draw_rect = 13,
    get_screen_size = 14,
    get_button = 17,
    system = 18,
    screen_put_image = 25,
    system_get = 26,
    get_sys_date = 29,
    mouse_get = 37,
    set_events_mask = 40,
    style_settings = 48,
    create_thread = 51,
    board = 63,
    keyboard = 66,
    change_window = 67,
    sys_misc = 68,
    file = 70,
    blitter = 73,
};

pub const Event = enum(u32) {
    none = 0,
    redraw = 1,
    key = 2,
    button = 3,
    background = 5,
    mouse = 6,
    ipc = 7,
};

pub inline fn syscall0(number: SYS) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
    );
}

pub inline fn syscall1(number: SYS, arg1: u32) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
          [arg1] "{ebx}" (arg1),
    );
}

pub inline fn syscall2(number: SYS, arg1: u32, arg2: u32) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
          [arg1] "{ebx}" (arg1),
          [arg2] "{ecx}" (arg2),
    );
}

pub inline fn syscall3(number: SYS, arg1: u32, arg2: u32, arg3: u32) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
          [arg1] "{ebx}" (arg1),
          [arg2] "{ecx}" (arg2),
          [arg3] "{edx}" (arg3),
    );
}

pub inline fn syscall4(number: SYS, arg1: u32, arg2: u32, arg3: u32, arg4: u32) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
          [arg1] "{ebx}" (arg1),
          [arg2] "{ecx}" (arg2),
          [arg3] "{edx}" (arg3),
          [arg4] "{esi}" (arg4),
    );
}

pub inline fn syscall5(number: SYS, arg1: u32, arg2: u32, arg3: u32, arg4: u32, arg5: u32) u32 {
    return asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (@intFromEnum(number)),
          [arg1] "{ebx}" (arg1),
          [arg2] "{ecx}" (arg2),
          [arg3] "{edx}" (arg3),
          [arg4] "{esi}" (arg4),
          [arg5] "{edi}" (arg5),
    );
}

pub fn terminateProcess() noreturn {
    _ = syscall0(SYS.terminate_process);
    unreachable;
}

pub const WindowFlags = struct {
    skinned: bool = true,
    fixed: bool = true,
    no_title: bool = false,
    relative_coordinates: bool = false,
    no_fill: bool = false,
    unmovable: bool = false,
};

pub fn createWindow(x: u16, y: u16, width: u16, height: u16, bgcolor: u24, flags: WindowFlags, title: [*:0]const u8) void {
    var f1: u32 = 0x00000000;
    if (flags.no_fill)
        f1 |= 0x40000000;
    if (flags.relative_coordinates)
        f1 |= 0x20000000;
    if (!flags.no_title)
        f1 |= 0x10000000;
    if (flags.skinned) {
        if (flags.fixed) {
            f1 |= 0x04000000;
        } else {
            f1 |= 0x03000000;
        }
    } else {
        f1 |= 0x01000000;
    }
    var f2: u32 = 0x00000000;
    if (flags.unmovable)
        f2 = 0x01000000;
    _ = syscall5(SYS.create_window, @as(u32, x) * 0x10000 + width, @as(u32, y) * 0x10000 + height, f1 | @as(u32, bgcolor), f2 | @as(u32, bgcolor), @intFromPtr(title));
}

pub fn putPixel(x: u16, y: u16, color: u24) void {
    _ = syscall3(SYS.put_pixel, x, y, color);
}

pub fn invertPixel(x: u16, y: u16) void {
    _ = syscall3(SYS.put_pixel, x, y, 0x01000000);
}

pub const Key = packed struct(u32) {
    _unused: u8 = 0,
    key: u8 = 0,
    scancode: u8 = 0,
    empty: u8 = 1,

    pub fn pressed(self: *const Key) bool {
        return self.key & 0x80 > 0;
    }
};

pub fn getKey() Key {
    return @bitCast(syscall0(SYS.get_key));
}

pub fn getSysTime() u24 {
    return @intCast(syscall0(SYS.get_sys_time));
}

pub fn getButton() u32 {
    return syscall0(SYS.get_button);
}

pub fn terminateThreadId(id: u32) void {
    _ = syscall2(SYS.system, 18, id);
}

pub fn drawText(x: u16, y: u16, color: u24, text: [*:0]const u8) void {
    _ = syscall5(SYS.draw_text, @as(u32, x) * 0x10000 + y, 0x80000000 | @as(u32, color), @intFromPtr(text), 0, 0);
}

pub fn sleep(centisecond: u32) void {
    _ = syscall1(SYS.sleep, centisecond);
}

pub fn beginDraw() void {
    _ = syscall1(SYS.redraw, 1);
}

pub fn endDraw() void {
    _ = syscall1(SYS.redraw, 2);
}

pub fn putImage(image: [*]const u8, width: u16, height: u16, x: u16, y: u16) void {
    _ = syscall3(SYS.put_image, @intFromPtr(image), @as(u32, width) * 0x10000 + height, @as(u32, x) * 0x10000 + y);
}

pub fn drawRect(x: u16, y: u16, width: u16, height: u16, color: u24) void {
    _ = syscall3(SYS.draw_rect, @as(u32, x) * 0x10000 + width, @as(u32, y) * 0x10000 + height, color);
}

pub fn getScreenSize() packed struct(u32) { height: u16, width: u16 } {
    return @bitCast(syscall0(SYS.get_screen_size));
}

pub fn waitEvent() Event {
    return @enumFromInt(syscall0(SYS.wait_event));
}

pub fn checkEvent() Event {
    return @enumFromInt(syscall0(SYS.check_event));
}

pub fn createThread(entry: *const fn () void, stack: []u8) u32 {
    return syscall3(SYS.create_thread, 1, @intFromPtr(entry), @intFromPtr(stack.ptr) + stack.len);
}

pub fn debugWrite(byte: u8) void {
    _ = syscall2(SYS.board, 1, byte);
}

pub fn debugWriteText(bytes: []const u8) void {
    for (bytes) |byte| {
        debugWrite(byte);
    }
}

pub const EventsMask = packed struct(u32) {
    redraw: bool = true, // 0
    key: bool = true,
    button: bool = true,
    _reserved: bool = false,
    background: bool = false,
    mouse: bool = false,
    ipc: bool = false,
    network: bool = false,
    debug: bool = false,
    _unused: u23 = 0,
};

pub fn setEventsMask(mask: EventsMask) EventsMask {
    return @bitCast(syscall1(SYS.set_events_mask, @bitCast(mask)));
}

pub fn getSkinHeight() u16 {
    return @intCast(syscall1(SYS.style_settings, 4));
}

pub fn screenPutImage(image: [*]const u32, width: u16, height: u16, x: u16, y: u16) void {
    _ = syscall3(SYS.screen_put_image, @intFromPtr(image), @as(u32, width) * 0x10000 + height, @as(u32, x) * 0x10000 + y);
}

pub fn systemGetTimeCount() u32 {
    return syscall1(SYS.system_get, 9);
}

pub fn getSysDate() u24 {
    return @intCast(syscall0(SYS.get_sys_date));
}

pub fn mouseGetScreenPosition() packed struct(u32) { y: u16, x: u16 } {
    return @bitCast(syscall1(SYS.mouse_get, 0));
}

pub fn mouseGetWindowPosition() packed struct(u32) { y: u16, x: u16 } {
    return @bitCast(syscall1(SYS.mouse_get, 1));
}

pub fn loadCursorIndirect(image: *const [32 * 32]u32, spotx: u5, spoty: u5) u32 {
    return syscall3(SYS.mouse_get, 4, @intFromPtr(image), 0x0002 | (@as(u32, spotx) << 24) | (@as(u32, spoty) << 16));
}

pub fn setCursor(cursor: u32) u32 {
    return syscall2(SYS.mouse_get, 5, cursor);
}

pub const MouseEvents = packed struct(u32) {
    left_hold: bool = false,
    right_hold: bool = false,
    middle_hold: bool = false,
    button4_hold: bool = false,
    button5_hold: bool = false,
    _unused0: u3 = 0,
    left_pressed: bool = false,
    right_pressed: bool = false,
    middle_pressed: bool = false,
    _unused1: u4 = 0,
    vertical_scroll: bool = false,
    left_released: bool = false,
    right_released: bool = false,
    middle_released: bool = false,
    _unused2: u4 = 0,
    horizontal_scroll: bool = false,
    left_double_clicked: bool = false,
    _unused3: u7 = 0,
};

pub fn mouseGetEvents() MouseEvents {
    return @bitCast(syscall1(SYS.mouse_get, 3));
}

pub fn heapInit() u32 {
    return syscall1(SYS.sys_misc, 11);
}

pub fn memAlloc(size: u32) *anyopaque {
    return @ptrFromInt(syscall2(SYS.sys_misc, 12, size));
}

pub fn memFree(ptr: *anyopaque) void {
    _ = syscall2(SYS.sys_misc, 13, @intFromPtr(ptr));
}

pub fn memRealloc(size: u32, ptr: *anyopaque) *anyopaque {
    return @ptrFromInt(syscall3(SYS.sys_misc, 20, size, @intFromPtr(ptr)));
}

fn alloc(ctx: *anyopaque, len: usize, alignment: std.mem.Alignment, ret_addr: usize) ?[*]u8 {
    _ = ctx;
    _ = alignment;
    _ = ret_addr;
    return @ptrCast(memAlloc(len));
}

fn free(ctx: *anyopaque, memory: []u8, alignment: std.mem.Alignment, ret_addr: usize) void {
    _ = ctx;
    _ = alignment;
    _ = ret_addr;
    memFree(@ptrCast(memory.ptr));
}

fn resize(ctx: *anyopaque, memory: []u8, alignment: std.mem.Alignment, new_len: usize, ret_addr: usize) bool {
    _ = ctx;
    _ = alignment;
    _ = ret_addr;
    _ = memRealloc(new_len, @ptrCast(memory.ptr));
    return true;
}

fn remap(ctx: *anyopaque, memory: []u8, alignment: std.mem.Alignment, new_len: usize, ret_addr: usize) ?[*]u8 {
    _ = ctx;
    _ = memory;
    _ = alignment;
    _ = new_len;
    _ = ret_addr;
    return null;
}

pub const allocator: std.mem.Allocator = .{
    .ptr = undefined,
    .vtable = &.{
        .alloc = alloc,
        .free = free,
        .resize = resize,
        .remap = remap,
    },
};

pub fn loadDriver(name: [*:0]const u8) u32 {
    return syscall2(SYS.sys_misc, 16, @intFromPtr(name));
}

pub fn controlDriver(drv: u32, func: u32, in: ?[]const u32, out: ?[]const *anyopaque) u32 {
    const ioctl: packed struct(u192) {
        drv: u32,
        func: u32,
        inptr: u32,
        insize: u32,
        outptr: u32,
        outsize: u32,
    } = .{
        .drv = drv,
        .func = func,
        .inptr = if (in) |v| @intFromPtr(v.ptr) else 0,
        .insize = if (in) |v| v.len * 4 else 0,
        .outptr = if (out) |v| @intFromPtr(v.ptr) else 0,
        .outsize = if (out) |v| v.len * 4 else 0,
    };
    return syscall2(SYS.sys_misc, 17, @intFromPtr(&ioctl));
}

pub const Signal = packed struct(u192) {
    kind: u32,
    data0: u32,
    data1: u32,
    data2: u32,
    data3: u32,
    data4: u32,
};

pub fn waitSignal(sig: *Signal) void {
    _ = syscall2(SYS.sys_misc, 14, @intFromPtr(sig));
}

pub const Sound = struct {
    drv: u32,

    pub const Buffer = struct {
        drv: u32,
        handle: u32,

        pub fn play(self: *const Buffer, flags: u32) void {
            _ = controlDriver(self.drv, 10, &.{ self.handle, flags }, null);
        }

        pub fn set(self: *const Buffer, src: []u8, offset: u32) void {
            _ = controlDriver(self.drv, 8, &.{ self.handle, @intFromPtr(src.ptr), offset, src.len }, null);
        }
    };

    pub fn init() Sound {
        return .{
            .drv = loadDriver("INFINITY"),
        };
    }

    pub fn createBuffer(self: *const Sound, format: u32, size: u32) Buffer {
        var ret: u32 = 0;
        _ = controlDriver(self.drv, 1, &.{ format, size }, &.{&ret});
        return .{
            .drv = self.drv,
            .handle = ret,
        };
    }
};

pub const InputMode = enum(u32) {
    normal = 0,
    scancodes = 1,
};

pub fn setInputMode(mode: InputMode) void {
    _ = syscall2(SYS.keyboard, 1, @intFromEnum(mode));
}

pub fn changeWindow(x: u32, y: u32, width: u32, height: u32) void {
    _ = syscall4(SYS.change_window, x, y, width, height);
}

pub const ControlKeys = packed struct(u32) {
    left_shift: bool,
    right_shift: bool,
    left_ctrl: bool,
    right_ctrl: bool,
    left_alt: bool,
    right_alt: bool,
    caps_lock: bool,
    num_lock: bool,
    scroll_lock: bool,
    left_win: bool,
    right_win: bool,
    _unused: u21,
};

pub fn getControlKeys() ControlKeys {
    return @bitCast(syscall1(SYS.keyboard, 3));
}

const FileInfo = packed struct(u200) {
    subfn: u32,
    offset: u64,
    size: u32,
    buffer: u32,
    path0: u8 = 0,
    pathptr: *const u8,
};

pub fn readFile(pathname: [*:0]const u8, offset: u64, buffer: []u8) !u32 {
    const info: FileInfo = .{
        .subfn = 0,
        .offset = offset,
        .size = buffer.len,
        .buffer = @intFromPtr(buffer.ptr),
        .pathptr = @ptrCast(pathname),
    };
    const err = asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (SYS.file),
          [info] "{ebx}" (&info),
    );
    const size = asm volatile (""
        : [ret] "={ebx}" (-> u32),
    );
    return switch (err) {
        0 => size,
        10 => error.AccessDenied,
        6 => size,
        else => error.Unexpected,
    };
}

pub fn createFile(pathname: [*:0]const u8, buffer: []u8) !u32 {
    const info: FileInfo = .{
        .subfn = 2,
        .offset = 0,
        .size = buffer.len,
        .buffer = @intFromPtr(buffer.ptr),
        .pathptr = @ptrCast(pathname),
    };
    const err = asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (SYS.file),
          [info] "{ebx}" (&info),
    );
    const size = asm volatile (""
        : [ret] "={ebx}" (-> u32),
    );
    return switch (err) {
        0 => size,
        10 => error.AccessDenied,
        8 => size,
        else => error.Unexpected,
    };
}

pub fn writeFile(pathname: [*:0]const u8, offset: u64, buffer: []u8) !u32 {
    const info: FileInfo = .{
        .subfn = 3,
        .offset = offset,
        .size = buffer.len,
        .buffer = @intFromPtr(buffer.ptr),
        .pathptr = @ptrCast(pathname),
    };
    const err = asm volatile ("int $0x40"
        : [ret] "={eax}" (-> u32),
        : [number] "{eax}" (SYS.file),
          [info] "{ebx}" (&info),
    );
    const size = asm volatile (""
        : [ret] "={ebx}" (-> u32),
    );
    return switch (err) {
        0 => size,
        10 => error.AccessDenied,
        5 => error.FileNotFound,
        else => error.Unexpected,
    };
}

pub fn fileGetSize(pathname: [*:0]const u8) !u64 {
    var ret: [10]u32 = undefined;
    const info: FileInfo = .{
        .subfn = 5,
        .offset = 0,
        .size = 0,
        .buffer = @intFromPtr(&ret),
        .pathptr = @ptrCast(pathname),
    };
    if (syscall1(SYS.file, @intFromPtr(&info)) != 0)
        return error.Unexpected;
    return ret[8] | (@as(u64, ret[9]) << 32);
}

pub fn deleteFile(pathname: [*:0]const u8) !void {
    const info: FileInfo = .{
        .subfn = 8,
        .offset = 0,
        .size = 0,
        .buffer = 0,
        .pathptr = @ptrCast(pathname),
    };
    const err = syscall1(SYS.file, @intFromPtr(&info));
    if (err != 0)
        return error.Unexpected;
}

pub const File = struct {
    pathname: [*:0]const u8,
    offset: u64 = 0,

    pub fn reader(file: *File) std.io.GenericReader(*File, anyerror, struct {
        fn read(context: *File, buffer: []u8) !usize {
            const size = try readFile(context.pathname, context.offset, buffer);
            context.offset += size;
            return size;
        }
    }.read) {
        return .{ .context = file };
    }
};

pub const BlitterFlags = packed struct(u32) {
    rop: u4 = 0,
    background: bool = false,
    transparent: bool = false,
    reserved1: u23 = 0,
    client_relative: bool = true,
    reserved2: u2 = 0,
};

pub fn blitter(dstx: u32, dsty: u32, dstw: u32, dsth: u32, srcx: u32, srcy: u32, srcw: u32, srch: u32, src: *const u8, pitch: u32, flags: BlitterFlags) void {
    _ = syscall2(SYS.blitter, @bitCast(flags), @intFromPtr(&[_]u32{ dstx, dsty, dstw, dsth, srcx, srcy, srcw, srch, @intFromPtr(src), pitch }));
}

pub const DebugWriter = std.io.GenericWriter(void, anyerror, struct {
    fn write(context: void, bytes: []const u8) !usize {
        _ = context;
        debugWriteText(bytes);
        return bytes.len;
    }
}.write);

pub const debug_writer: DebugWriter = .{ .context = {} };
