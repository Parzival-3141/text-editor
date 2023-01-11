const std = @import("std");
const Builder = std.build.Builder;

pub fn build(b: *Builder) void {
    const target = b.standardTargetOptions(.{});
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("wheel", null);
    exe.setTarget(target);
    exe.setBuildMode(mode);
    exe.linkLibC();
    exe.install();

    const csources = get_c_sourcefiles(b, "src") catch unreachable;
    defer csources.deinit();

    exe.addCSourceFiles(csources.items, &.{ "-Wall", "-Wextra", "-pedantic", "--debug", "-Wno-unused-parameter" });

    linkSDL2(b, exe);
    linkFreetype(b, exe);

    b.installBinFile("deps/Hack Regular Nerd Font Complete.ttf", "Hack Regular Nerd Font Complete.ttf");

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

fn get_c_sourcefiles(builder: *Builder, search_dir: []const u8) !std.ArrayList([]const u8) {
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

fn linkSDL2(b: *Builder, step: *std.build.LibExeObjStep) void {
    step.addIncludePath("deps/SDL2/include/SDL2");
    step.addLibraryPath("deps/SDL2/lib");
    step.linkSystemLibraryName("SDL2");

    // @Todo: dynamic lib crossplatform support
    b.installBinFile("deps/SDL2/lib/SDL2.dll", "SDL2.dll");
}

// Freetype build stuff
// credit to https://github.com/hexops/mach-freetype/blob/main/build.zig
const ft_root = "./deps/freetype/freetype";
const ft_include_path = ft_root ++ "/include";

fn linkFreetype(b: *Builder, step: *std.build.LibExeObjStep) void {
    const ft_lib = buildFreetype(b, step.build_mode, step.target);
    step.linkLibrary(ft_lib);
    step.addIncludePath(ft_include_path);

    std.debug.print("Freetype2 Done\n", .{});
}

fn buildFreetype(b: *Builder, mode: std.builtin.Mode, target: std.zig.CrossTarget) *std.build.LibExeObjStep {
    std.debug.print("Building Freetype2...\n", .{});

    const lib = b.addStaticLibrary("freetype", null);
    lib.defineCMacro("FT2_BUILD_LIBRARY", "1");
    lib.setBuildMode(mode);
    lib.setTarget(target);
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
