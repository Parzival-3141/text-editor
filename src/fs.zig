const std = @import("std");
const fs = std.fs;
const log = std.log;
const ArenaAllocator = std.heap.ArenaAllocator;
const ArrayList = std.ArrayList;

// const c = @cImport({
//     @cInclude("cglm/cglm.h");
// });

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

export fn FS_init(path: [*:0]const u8) void {
    fsys = .{
        .current_path = std.mem.span(path),
        .nodes = ArrayList(FS_Node).init(arena.allocator()),
    };
}

export fn FS_deinit() void {
    fsys.nodes.deinit();
    arena.deinit();
    _ = gpa.deinit();
}

/// Returns true if unable to open directory at fsys.current_path.
/// Invalidates pointers!
export fn FS_read_directory() bool {
    fsys.nodes.clearAndFree();
    get_nodes() catch |err| {
        log.err("Unable to read directory '{s}': {s}", .{ fsys.current_path, @errorName(err) });
        return true;
    };

    return false;
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

fn safe_iterable_dir_next(iter: *fs.IterableDir.Iterator) ?fs.IterableDir.Entry {
    return iter.next() catch safe_iterable_dir_next(iter); // @Todo: Possible stack overflow?
    // return iter.next() catch |err| {
    //     log.err("Unable to read dir entry: {s}", .{@errorName(err)});
    //     return null;
    // };
}

fn log_err(comptime fmt: []const u8, args: anytype) noreturn {
    log.err(fmt, args);
    std.os.exit(1);
}

fn get_nodes() fs.Dir.OpenError!void {
    var iterable_dir = try fs.cwd().openIterableDir(fsys.current_path, .{});
    defer iterable_dir.close();

    const allocator = arena.allocator();
    var iter = iterable_dir.iterate();

    while (safe_iterable_dir_next(&iter)) |entry| {
        const tag: FS_NodeType = switch (entry.kind) {
            .Directory => .directory,
            .File => .file,
            else => continue,
        };

        const name = allocator.alloc(u8, entry.name.len + 1) catch log_err("OOM", .{});
        for (entry.name, 0..) |char, i| {
            name[i] = char;
        }
        name[name.len - 1] = 0;
        // const name = allocator.dupe(u8, entry.name) catch log_err("OOM", .{});
        fsys.nodes.append(.{
            .name = @ptrCast([*:0]const u8, name),
            .type = tag,
        }) catch log_err("OOM", .{});
    }
}
