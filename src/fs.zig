const std = @import("std");
const fs = std.fs;
const log = std.log;
const math = std.math;
const ArenaAllocator = std.heap.ArenaAllocator;
const ArrayList = std.ArrayList;

const c = @cImport({
    @cInclude("cglm/cglm.h");
});

// Abstract file system access
// Expose a list of nodes that contain file/folder info
// for the current directory.

// In C iterate over the list and draw them.

const FS_NodeType = enum(c_int) {
    file,
    directory,
};

const FS_Node = extern struct {
    name: [*:0]const u8,
    pos: c.vec2,
    type: FS_NodeType,
};

const FileSystem = struct {
    current_path: []const u8,
    nodes: ArrayList(FS_Node),
};

var fsys: FileSystem = undefined;
var gpa = std.heap.GeneralPurposeAllocator(.{}){};
var arena = std.heap.ArenaAllocator.init(gpa.allocator());
// var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);

/// Returns false if path is invalid
export fn FS_init(path: [*:0]const u8) bool {
    const realpath = fs.realpathAlloc(arena.allocator(), std.mem.span(path)) catch |err| {
        log.err("Cannot init FileSystem! Invalid path '{s}': {s}", .{ path, @errorName(err) });
        return false;
    };

    fsys = .{
        .current_path = realpath,
        .nodes = ArrayList(FS_Node).init(arena.allocator()),
    };

    return true;
}

export fn FS_deinit() void {
    fsys.nodes.deinit();
    arena.deinit();
    _ = gpa.deinit();
}

/// Returns false if unable to open directory at fsys.current_path.
/// Invalidates pointers!
export fn FS_read_directory() bool {
    fsys.nodes.clearAndFree();
    get_nodes() catch |err| {
        log.err("Unable to read directory '{s}': {s}", .{ fsys.current_path, @errorName(err) });
        return false;
    };

    return true;
}

export fn FS_view_nodes() [*]FS_Node {
    return fsys.nodes.items.ptr;
}

export fn FS_nodes_length() usize {
    return fsys.nodes.items.len;
}

export fn FS_get_nodetype_as_cstr(ntype: FS_NodeType) [*:0]const u8 {
    return @tagName(ntype);
}

/// returns true on success
export fn FS_get_node_at_position(x: f32, y: f32, out_node: **FS_Node) bool {
    // Nodes are 100x100
    for (fsys.nodes.items) |*node| {
        if ((x >= node.pos[0] and x < node.pos[0] + 100) and (y >= node.pos[1] and y < node.pos[1] + 100)) {
            out_node.* = node;
            return true;
        }
    }

    return false;
}

/// returns true on success
export fn FS_cd(path: [*:0]const u8) bool {
    const allocator = arena.allocator();

    const new_path = blk: {
        const path_slice = std.mem.span(path);

        if (fs.path.isAbsolute(path_slice)) {
            break :blk allocator.dupe(u8, path_slice) catch @panic("OOM");
        } else {
            break :blk fs.path.join(allocator, &[_][]const u8{ fsys.current_path, path_slice }) catch @panic("OOM");
        }
    };

    fs.cwd().access(new_path, .{}) catch |err| {
        log.err("Unable to cd into {s}: {s}", .{ new_path, @errorName(err) });
        allocator.free(new_path);
        return false;
    };

    allocator.free(fsys.current_path);
    fsys.current_path = new_path;
    return true;
}

const MAX_FILE_BYTES = 1e+9; // 1 GB

/// returns true on success
export fn FS_open_file(name: [*:0]const u8, data: *[*:0]u8, size: *usize) bool {
    var dir = fs.cwd().openDir(fsys.current_path, .{}) catch |err| {
        // TODO handle case where directory is inaccessible
        log_fatal_err(
            "Unable to open directory '{s}': {s}\nThe directory probably doesn't exist.",
            .{ fsys.current_path, @errorName(err) },
        );
    };
    defer dir.close();

    const file = dir.openFile(std.mem.span(name), .{}) catch |err| {
        log.err("Unable to open '{s}/{s}': {s}", .{ fsys.current_path, name, @errorName(err) });
        return false;
    };
    defer file.close();

    const bytes = file.readToEndAllocOptions(std.heap.raw_c_allocator, MAX_FILE_BYTES, null, @alignOf(u8), 0) catch |err| {
        switch (err) {
            error.FileTooBig => log.err(
                "Unable to open '{s}/{s}': Cannot open file bigger than 1GB!",
                .{ fsys.current_path, name },
            ),
            else => log.err("Unable to open '{s}/{s}': {s}", .{ fsys.current_path, name, @errorName(err) }),
        }
        return false;
    };

    data.* = bytes.ptr;
    size.* = bytes.len;
    return true;
}

fn safe_iterable_dir_next(iter: *fs.IterableDir.Iterator) ?fs.IterableDir.Entry {
    return iter.next() catch safe_iterable_dir_next(iter); // @Todo: Possible stack overflow?
    // return iter.next() catch |err| {
    //     log.err("Unable to read dir entry: {s}", .{@errorName(err)});
    //     return null;
    // };
}

fn log_fatal_err(comptime fmt: []const u8, args: anytype) noreturn {
    log.err(fmt, args);
    std.os.exit(1);
}

fn get_nodes() fs.Dir.OpenError!void {
    var iterable_dir = try fs.cwd().openIterableDir(fsys.current_path, .{});
    defer iterable_dir.close();

    const allocator = arena.allocator();
    var iter = iterable_dir.iterate();

    var spiral_n: f32 = 0;
    while (safe_iterable_dir_next(&iter)) |entry| : (spiral_n += 1) {
        const tag: FS_NodeType = switch (entry.kind) {
            .Directory => .directory,
            .File => .file,
            else => continue,
        };

        const name = allocator.alloc(u8, entry.name.len + 1) catch log_fatal_err("OOM", .{});
        for (entry.name, 0..) |char, j| {
            name[j] = char;
        }
        name[name.len - 1] = 0;
        // const name = allocator.dupe(u8, entry.name) catch log_fatal_err("OOM", .{});
        fsys.nodes.append(.{
            .name = @ptrCast([*:0]const u8, name),
            .pos = if (@mod(spiral_n, 2) == 0) fermat_spiral(math.max(0, spiral_n - 1), true) else fermat_spiral(spiral_n, false),
            // .pos = fermat_golden_ratio_spiral(spiral_n),
            .type = tag,
        }) catch log_fatal_err("OOM", .{});
    }
}

const spiral_scale = 200; // nodes are 100x100 pixels

fn fermat_golden_ratio_spiral(n: f32) c.vec2 {
    const theta = n * 137.508; // The angle 137.508° is the golden angle which is approximated by ratios of Fibonacci numbers.
    const radius = spiral_scale * math.sqrt(n);
    return c.vec2{ radius * math.cos(theta), radius * math.sin(theta) };
}

fn fermat_spiral(t: f32, inverse: bool) c.vec2 {
    // const b = spiral_scale * math.sqrt(t);
    const b = sign_from_bool(f32, !inverse) * spiral_scale * math.sqrt(t);
    return c.vec2{ b * math.cos(t), b * math.sin(t) };
}

fn sign_from_bool(comptime T: anytype, b: bool) T {
    if (b) return @as(T, 1) else return @as(T, -1);
}
