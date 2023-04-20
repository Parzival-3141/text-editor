const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const exe = b.addExecutable(.{ .name = "wheel", .target = target, .optimize = optimize });
    exe.linkLibC();
    b.installArtifact(exe);

    const csources = get_file_paths(b, "src", ".c") catch unreachable;
    defer csources.deinit();

    exe.addCSourceFiles(csources.items, &.{ "-Wall", "-Wextra", "-pedantic", "--debug", "-Werror", "-Wno-unused-parameter" });

    linkSDL2(b, exe);
    linkFreetype(b, exe);
    link_glad(b, exe);
    exe.addIncludePath("deps/cglm/include");

    b.installBinFile("assets/Hack Regular Nerd Font Complete.ttf", "assets/Hack Regular Nerd Font Complete.ttf");

    const shaders = get_file_paths(b, "assets/shaders", null) catch unreachable;
    defer shaders.deinit();

    // @Note: Obtained from https://fonts.google.com/icons
    const icons = get_file_paths(b, "assets/icons", null) catch unreachable;
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

fn get_file_paths(b: *std.Build, search_dir: []const u8, extension: ?[]const u8) !std.ArrayList([]const u8) {
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

fn linkSDL2(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    step.addIncludePath("deps/SDL2/include");
    step.addLibraryPath("deps/SDL2");
    step.linkSystemLibrary("SDL2");

    // @Todo: dynamic lib crossplatform support
    b.installBinFile("deps/SDL2/SDL2.dll", "SDL2.dll");
}

const glad_path = "deps/glad";
fn link_glad(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    const glad = build_glad(b, step.optimize, step.target);
    step.linkLibrary(glad);
    step.addIncludePath(glad_path);
}

fn build_glad(b: *std.Build, mode: std.builtin.Mode, target: std.zig.CrossTarget) *std.Build.LibExeObjStep {
    const lib = b.addStaticLibrary(.{ .name = "glad", .target = target, .optimize = mode });
    lib.linkLibC();

    lib.addIncludePath(glad_path);

    const glad_src = get_file_paths(b, glad_path, ".c") catch unreachable;
    lib.addCSourceFiles(glad_src.items, &.{});
    glad_src.deinit();

    return lib;
}

// Freetype build stuff
// credit to https://github.com/hexops/mach-freetype/blob/main/build.zig
const ft_root = "./deps/freetype/freetype";
const ft_include_path = ft_root ++ "/include";

fn linkFreetype(b: *std.Build, step: *std.Build.LibExeObjStep) void {
    const ft_lib = buildFreetype(b, step.optimize, step.target);
    step.linkLibrary(ft_lib);
    step.addIncludePath(ft_include_path);
}

fn buildFreetype(b: *std.Build, mode: std.builtin.Mode, target: std.zig.CrossTarget) *std.Build.LibExeObjStep {
    const lib = b.addStaticLibrary(.{
        .name = "freetype",
        .target = target,
        .optimize = mode,
    });

    lib.defineCMacro("FT2_BUILD_LIBRARY", "1");
    lib.linkLibC();
    lib.addIncludePath(ft_include_path);

    const target_info = (std.zig.system.NativeTargetInfo.detect(target) catch unreachable).target;

    if (target_info.os.tag == .windows) {
        lib.addCSourceFile(ft_root ++ "/builds/windows/ftsystem.c", &.{});
        lib.addCSourceFile(ft_root ++ "/builds/windows/ftdebug.c", &.{});
    } else {
        lib.addCSourceFile(ft_root ++ "/src/base/ftsystem.c", &.{});
        lib.addCSourceFile(ft_root ++ "/src/base/ftdebug.c", &.{});
    }
    if (target_info.os.tag.isBSD() or target_info.os.tag == .linux) {
        lib.defineCMacro("HAVE_UNISTD_H", "1");
        lib.defineCMacro("HAVE_FCNTL_H", "1");
        lib.addCSourceFile(ft_root ++ "/builds/unix/ftsystem.c", &.{});
        if (target_info.os.tag == .macos)
            lib.addCSourceFile(ft_root ++ "/src/base/ftmac.c", &.{});
    }

    lib.addCSourceFiles(freetype_base_sources, &.{});
    return lib;
}

const freetype_base_sources = &[_][]const u8{
    ft_root ++ "/src/autofit/autofit.c",
    ft_root ++ "/src/base/ftbase.c",
    ft_root ++ "/src/base/ftbbox.c",
    ft_root ++ "/src/base/ftbdf.c",
    ft_root ++ "/src/base/ftbitmap.c",
    ft_root ++ "/src/base/ftcid.c",
    ft_root ++ "/src/base/ftfstype.c",
    ft_root ++ "/src/base/ftgasp.c",
    ft_root ++ "/src/base/ftglyph.c",
    ft_root ++ "/src/base/ftgxval.c",
    ft_root ++ "/src/base/ftinit.c",
    ft_root ++ "/src/base/ftmm.c",
    ft_root ++ "/src/base/ftotval.c",
    ft_root ++ "/src/base/ftpatent.c",
    ft_root ++ "/src/base/ftpfr.c",
    ft_root ++ "/src/base/ftstroke.c",
    ft_root ++ "/src/base/ftsynth.c",
    ft_root ++ "/src/base/fttype1.c",
    ft_root ++ "/src/base/ftwinfnt.c",
    ft_root ++ "/src/bdf/bdf.c",
    ft_root ++ "/src/bzip2/ftbzip2.c",
    ft_root ++ "/src/cache/ftcache.c",
    ft_root ++ "/src/cff/cff.c",
    ft_root ++ "/src/cid/type1cid.c",
    ft_root ++ "/src/gzip/ftgzip.c",
    ft_root ++ "/src/lzw/ftlzw.c",
    ft_root ++ "/src/pcf/pcf.c",
    ft_root ++ "/src/pfr/pfr.c",
    ft_root ++ "/src/psaux/psaux.c",
    ft_root ++ "/src/pshinter/pshinter.c",
    ft_root ++ "/src/psnames/psnames.c",
    ft_root ++ "/src/raster/raster.c",
    ft_root ++ "/src/sdf/sdf.c",
    ft_root ++ "/src/sfnt/sfnt.c",
    ft_root ++ "/src/smooth/smooth.c",
    ft_root ++ "/src/svg/svg.c",
    ft_root ++ "/src/truetype/truetype.c",
    ft_root ++ "/src/type1/type1.c",
    ft_root ++ "/src/type42/type42.c",
    ft_root ++ "/src/winfonts/winfnt.c",
};
