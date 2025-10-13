// SPDX-FileCopyrightText: 2025 iyzsong@envs.net
//
// SPDX-License-Identifier: MPL-2.0

const std = @import("std");

pub fn build(b: *std.Build) void {
    const target_query = std.Target.Query{
        .cpu_arch = std.Target.Cpu.Arch.x86,
        .os_tag = std.Target.Os.Tag.freestanding,
        .abi = std.Target.Abi.none,
        .cpu_model = std.Target.Query.CpuModel{ .explicit = &std.Target.x86.cpu.i586 },
    };
    const target = b.resolveTargetQuery(target_query);
    const optimize = b.standardOptimizeOption(.{});
    const zuxn = b.dependency("zuxn", .{
        .target = target,
        .optimize = optimize,
    });
    const elf = b.addExecutable(.{
        .name = "uxn.elf",
        .root_source_file = b.path("src/main.zig"),
        .target = target,
        .optimize = optimize,
        .unwind_tables = .none,
    });
    elf.root_module.addImport("uxn-core", zuxn.module("uxn-core"));
    elf.root_module.addImport("uxn-varvara", zuxn.module("uxn-varvara"));
    elf.setLinkerScript(b.path("src/linker.ld"));
    const bin = elf.addObjCopy(.{
        .format = .bin,
    });
    const install_bin = b.addInstallBinFile(bin.getOutput(), "uxn");
    b.getInstallStep().dependOn(&install_bin.step);
    b.installArtifact(elf);
}
