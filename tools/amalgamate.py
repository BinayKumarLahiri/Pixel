import os
import re
import sys

IMPLEMENTATION_SUFFIX = "_IMPLEMENTATION"

def generate_macro_name(filename, suffix=""):
    # Converts: pixel.h into valid preprocessor identifiers: PIXEL_H
    base = os.path.basename(filename).upper()
    macro = re.sub(r"[^A-Z0-9_]", "_", base)
    if macro and macro[0].isdigit():
        macro = "_" + macro

    return f"{macro}{suffix}"

def read_file(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        return f.read()

def normalize_path(path):
    return os.path.normcase(os.path.normpath(os.path.abspath(path)))


LOCAL_INCLUDE_PATTERN = re.compile(
    r'^\s*#\s*include\s+"([^"]+)"\s*$'
)

def find_local_include(
    including_file,
    include_name,
    known_files
):
    """
    Resolve: #include "foo.h"
    relative to the file containing the include.
    We also check the source directory tree.
    Returns:
        normalized absolute path
        or None if it is not one of our files.
    """

    including_dir = os.path.dirname(including_file)

    # First: relative to the current file.
    candidate = normalize_path(
        os.path.join(including_dir, include_name)
    )
    if candidate in known_files:
        return candidate
    # Second: search by relative path from source root.
    basename = os.path.basename(include_name)

    for known_file in known_files:
        if os.path.basename(known_file) == basename:
            return known_file

    return None


def strip_local_includes(
    content,
    filepath,
    known_files
):
    """
    Remove includes referring to files that are beingamalgamated. External includes remain untouched.
    Example:
        #include "pixel.h"
        #include "foo.h"
        #include <cmath>
    becomes:
        #include <cmath>
    """

    result = []
    for line in content.splitlines(keepends=True):
        match = LOCAL_INCLUDE_PATTERN.match(line)
        if match:
            include_name = match.group(1)
            included_file = find_local_include(
                filepath,
                include_name,
                known_files
            )

            if included_file is not None:
                # This is one of our own files.
                continue

        result.append(line)
    return "".join(result)

def remove_header_guard(content):
    """
    Remove the conventional include guard from the original public header.
    Example:
        #ifndef PIXEL_H
        #define PIXEL_H
        ... declarations ...
        #endif
    becomes:
        ... declarations ...
    This is important because the amalgamated file provides
    its OWN outer include guard.
    """

    lines = content.splitlines(keepends=True)
    if len(lines) < 3:
        return content
    first = 0
    while first < len(lines) and not lines[first].strip():
        first += 1
    if first >= len(lines):
        return content
    match = re.match(
        r"^\s*#\s*ifndef\s+([A-Za-z_][A-Za-z0-9_]*)",
        lines[first]
    )
    if not match:
        return content

    guard = match.group(1)
    define = first + 1
    while define < len(lines) and not lines[define].strip():
        define += 1

    if define >= len(lines):
        return content

    define_match = re.match(
        rf"^\s*#\s*define\s+{re.escape(guard)}\s*$",
        lines[define]
    )

    if not define_match:
        return content

    lines = lines[:first] + lines[define + 1:]
    end_index = None

    for i in range(len(lines) - 1, -1, -1):
        if re.match(
            r"^\s*#\s*endif\b",
            lines[i]
        ):
            end_index = i
            break

    if end_index is not None:
        lines.pop(end_index)

    return "".join(lines)

def discover_files(src_dir):
    header_files = []
    source_files = []

    for root, _, files in os.walk(src_dir):
        for filename in files:
            filepath = normalize_path(
                os.path.join(root, filename)
            )
            ext = os.path.splitext(filename)[1].lower()
            if ext in {".h", ".hpp"}:
                header_files.append(filepath)
            elif ext in {".c", ".cc", ".cpp", ".cxx"}:
                source_files.append(filepath)

    header_files.sort()
    source_files.sort()

    return header_files, source_files

def find_public_header(
    header_files,
    output_file
):
    """Pick the public header. For Pixel, this means pixel.h.
    If there is only one header, use it automatically.
    """

    if len(header_files) == 1:
        return header_files[0]

    output_base = os.path.splitext(
        os.path.basename(output_file)
    )[0].lower()

    # Prefer a header matching the output name.
    for header in header_files:

        header_base = os.path.splitext(
            os.path.basename(header)
        )[0].lower()

        if header_base == output_base:
            return header

    # Prefer pixel.h if present.
    for header in header_files:

        if os.path.basename(header).lower() == "pixel.h":
            return header

    raise RuntimeError(
        "Multiple headers found, but could not determine "
        "the public header.\n"
        "Use a header whose basename matches the output "
        "filename or name it pixel.h."
    )


# ==============================================================
# Generate STB-style header
# ==============================================================

def amalgamate(src_dir, output_file):

    src_dir = normalize_path(src_dir)
    output_file = normalize_path(output_file)

    header_files, source_files = discover_files(src_dir)

    if not header_files:
        raise RuntimeError(
            f"No header files found in: {src_dir}"
        )

    public_header = find_public_header(
        header_files,
        output_file
    )

    # All files belonging to the project.
    all_project_files = {
        normalize_path(path)
        for path in header_files + source_files
    }
    # Generate macro names
    guard_macro = generate_macro_name(
        output_file
    )

    implementation_macro = generate_macro_name(
        output_file,
        IMPLEMENTATION_SUFFIX
    )
    # Create output directory
    output_dir = os.path.dirname(output_file)

    if output_dir:
        os.makedirs(
            output_dir,
            exist_ok=True
        )
    # Generate output

    with open(
        output_file,
        "w",
        encoding="utf-8",
        newline="\n"
    ) as out:
        # File header
        out.write(
            "/*\n"
            " * ============================================================\n"
            " *  Automatically generated STB-style amalgamated header.\n"
            " *\n"
            " *  DO NOT EDIT THIS FILE DIRECTLY.\n"
            " *\n"
            " *  Normal usage:\n"
            " *\n"
            " *      #include \"pixel.h\"\n"
            " *\n"
            " *  Exactly ONE source file must contain:\n"
            " *\n"
            f" *      #define {implementation_macro}\n"
            " *      #include \"pixel.h\"\n"
            " *\n"
            " * ============================================================\n"
            " */\n\n"
        )
        # Outer include guard
        out.write(
            f"#ifndef {guard_macro}\n"
            f"#define {guard_macro}\n\n"
        )
        # PUBLIC HEADER
        out.write(
            "/* ============================================================\n"
            " * PUBLIC API\n"
            " * ============================================================ */\n\n"
        )

        public_content = read_file(public_header)

        # Remove the original include guard.
        public_content = remove_header_guard(
            public_content
        )

        # Remove local includes.
        public_content = strip_local_includes(
            public_content,
            public_header,
            all_project_files
        )

        out.write(
            f"/* --- Begin {os.path.basename(public_header)} --- */\n\n"
        )

        out.write(public_content)

        out.write(
            f"\n/* --- End {os.path.basename(public_header)} --- */\n\n"
        )
        # IMPLEMENTATION
        out.write(
            "/* ============================================================\n"
            " * IMPLEMENTATION\n"
            " * ============================================================ */\n\n"
        )

        out.write(
            f"#ifdef {implementation_macro}\n\n"
        )
        # Add source files
        for source_file in source_files:

            # Don't accidentally amalgamate the generated file
            # if the output happens to be inside src/.
            if normalize_path(source_file) == output_file:
                continue

            content = read_file(source_file)

            # Remove project-local includes.
            content = strip_local_includes(
                content,
                source_file,
                all_project_files
            )

            out.write(
                f"/* --- Begin {os.path.basename(source_file)} --- */\n\n"
            )

            out.write(content)

            out.write(
                f"\n/* --- End {os.path.basename(source_file)} --- */\n\n"
            )
        # End implementation
        out.write(
            f"#endif /* {implementation_macro} */\n\n"
        )
        # End outer include guard
        out.write(
            f"#endif /* {guard_macro} */\n"
        )

    print(
        f"Successfully amalgamated:\n"
        f"  Public header : {os.path.basename(public_header)}\n"
        f"  Headers found : {len(header_files)}\n"
        f"  Sources found : {len(source_files)}\n"
        f"  Output        : {output_file}\n"
        f"  Guard         : {guard_macro}\n"
        f"  Implementation: {implementation_macro}"
    )

if __name__ == "__main__":

    if len(sys.argv) != 3:

        print(
            "Usage:\n"
            "    python amalgamate.py <source_directory> <output_file>\n\n"
            "Example:\n"
            "    python amalgamate.py src dist/pixel.h"
        )
        sys.exit(1)
    src_directory = sys.argv[1]
    output_filepath = sys.argv[2]

    try:
        amalgamate(src_directory,output_filepath)
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        sys.exit(1)
