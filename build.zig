const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{
        .name = "wheel",
        .target = target,
        .optimize = optimize,
    });
    exe.linkLibC();
    b.installArtifact(exe);

    const csources = get_project_filepaths(b, "src", ".c") catch unreachable;
    defer csources.deinit();

    exe.addCSourceFiles(csources.items, &.{ "-Wall", "-Wextra", "-pedantic", "--debug", "-Werror", "-Wno-unused-parameter" });

    const fs_obj = b.addObject(.{
        .name = "fs",
        .root_source_file = .{ .path = "src/fs.zig" },
        .target = target,
        .optimize = optimize,
    });
    fs_obj.linkLibC();
    fs_obj.addIncludePath("deps/cglm/include");
    // fs_obj.emit_h = true;
    exe.step.dependOn(&fs_obj.step);
    exe.addObject(fs_obj);

    addSDL2(b, exe);
    addFreetype(b, exe);
    addGlad(b, exe);
    exe.addIncludePath("deps/cglm/include");

    b.installBinFile("assets/Hack Regular Nerd Font Complete.ttf", "assets/Hack Regular Nerd Font Complete.ttf");

    const shaders = get_project_filepaths(b, "assets/shaders", null) catch unreachable;
    defer shaders.deinit();

    const icons = get_project_filepaths(b, "assets/icons", null) catch unreachable;
    defer icons.deinit();

    installBinFiles(b, shaders.items, "assets/shaders") catch unreachable;
    installBinFiles(b, icons.items, "assets/icons") catch unreachable;

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

fn installBinFiles(b: *std.Build, src_paths: [][]const u8, dest_relative_path: []const u8) !void {
    for (src_paths) |src| {
        const dest = try std.fs.path.join(b.allocator, &[_][]const u8{ dest_relative_path, std.fs.path.basename(src) });
        b.installBinFile(src, dest);
    }
}

fn get_project_filepaths(b: *std.Build, sub_dir: []const u8, extension: ?[]const u8) !std.ArrayList([]const u8) {
    return get_filepaths(b, b.pathFromRoot(sub_dir), extension);
}

fn get_filepaths(b: *std.Build, search_dir: []const u8, extension: ?[]const u8) !std.ArrayList([]const u8) {
    var paths = std.ArrayList([]const u8).init(b.allocator);

    var dir = try std.fs.cwd().openIterableDir(search_dir, .{ .access_sub_paths = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        if (entry.kind != .File) continue;

        if (extension) |ext| {
            if (!std.mem.eql(u8, std.fs.path.extension(entry.basename), ext))
                continue;
        }

        var path = try std.fs.path.join(b.allocator, &.{ search_dir, b.dupe(entry.path) });
        try paths.append(path);
    }

    return paths;
}

fn addSDL2(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    step.addIncludePath("deps/SDL2/include");
    step.addLibraryPath("deps/SDL2");
    step.linkSystemLibrary("SDL2");

    // @Todo: dynamic lib crossplatform support
    b.installBinFile("deps/SDL2/SDL2.dll", "SDL2.dll");
}

const glad_path = "deps/glad";
fn addGlad(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    const glad = build_glad(b, step.optimize, step.target);
    step.linkLibrary(glad);
    step.addIncludePath(glad_path);
}

fn build_glad(b: *std.Build, optimize: std.builtin.OptimizeMode, target: std.zig.CrossTarget) *std.Build.LibExeObjStep {
    const lib = b.addStaticLibrary(.{
        .name = "glad",
        .target = target,
        .optimize = optimize,
    });
    lib.linkLibC();
    lib.addIncludePath(glad_path);

    const glad_src = get_project_filepaths(b, glad_path, ".c") catch unreachable;
    lib.addCSourceFiles(glad_src.items, &.{});
    glad_src.deinit();

    return lib;
}

fn addFreetype(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    const ft = b.anonymousDependency("deps/freetype", @import("deps/freetype/build.zig"), .{});
    step.linkLibrary(ft.artifact("freetype"));
    step.addIncludePath("deps/freetype/include");
}
