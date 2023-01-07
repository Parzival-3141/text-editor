const std = @import("std");

const includes = "deps/include/";
const libs = "deps/lib/";

pub fn build(b: *std.build.Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("wheel", null);
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.linkLibC();
    exe.install();

    const csources = get_c_sourcefiles(b, "src") catch unreachable;
    defer csources.deinit();

    std.debug.print("Compiling source files: {s}\n", .{csources.items});
    exe.addCSourceFiles(csources.items, &.{});

    includeLibrary(exe, "SDL2");
    b.installBinFile(libs ++ "SDL2/SDL2.dll", "SDL2.dll");

    // includeLibrary(exe, "SDL2_ttf");
    // b.installBinFile(libs ++ "SDL2_ttf/SDL2_ttf.dll", "SDL2_ttf.dll");

    // Can't statically link unless I compile with zig cc/c++, because
    // for some reason you can't mix static libs from different compilers >:(
    //exe.addIncludePath(includes ++ "freetype");
    //exe.addObjectFile(libs ++ "freetype/freetype.lib");

    includeLibrary(exe, "freetype");
    b.installBinFile(libs ++ "freetype/freetype.dll", "freetype.dll");

    b.installBinFile("deps/Hack Regular Nerd Font Complete.ttf", "Hack Regular Nerd Font Complete.ttf");

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

fn includeLibrary(step: *std.build.LibExeObjStep, comptime dir_name: []const u8) void {
    step.addIncludePath(includes ++ dir_name);
    step.addLibraryPath(libs ++ dir_name);
    step.linkSystemLibraryName(dir_name);
}

fn get_c_sourcefiles(builder: *std.build.Builder, search_dir: []const u8) !std.ArrayList([]const u8) {
    var sources = std.ArrayList([]const u8).init(builder.allocator);

    var dir = try std.fs.cwd().openIterableDir(search_dir, .{ .access_sub_paths = true });
    defer dir.close();

    var walker = try dir.walk(builder.allocator);
    defer walker.deinit();

    while (try walker.next()) |entry| {
        const ext = std.fs.path.extension(entry.basename);
        if (std.mem.eql(u8, ext, ".c")) {
            var path = try std.fs.path.join(builder.allocator, &.{ search_dir, builder.dupe(entry.path) });
            try sources.append(path);
        }
    }

    return sources;
}
