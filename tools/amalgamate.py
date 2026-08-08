import os
import sys
import re

def generate_macro_name(filename, suffix=""):
    """Converts a filename like 'my_gfx.h' to 'MY_GFX_H'"""
    base = os.path.basename(filename).upper()
    macro = re.sub(r'[^A-Z0-9_]', '_', base)
    return f"{macro}{suffix}"

def amalgamate(src_dir, output_file):
    header_files = []
    source_files = []
    
    # 1. Catalog all files in the source directory
    for root, _, files in os.walk(src_dir):
        for file in files:
            ext = os.path.splitext(file)[1].lower()
            filepath = os.path.join(root, file)
            
            if ext in ['.h', '.hpp']:
                header_files.append(filepath)
            elif ext in ['.c', '.cpp', '.cc']:
                source_files.append(filepath)

    # Sort for deterministic output builds
    header_files.sort()
    source_files.sort()

    # Create a set of base filenames we are merging to know what to strip
    amalgamated_basenames = {os.path.basename(f) for f in (header_files + source_files)}
    
    # Regex to catch: #include "something.h"
    local_include_pattern = re.compile(r'^\s*#\s*include\s+"([^"]+)"')

    def process_file_contents(filepath):
        processed_lines = []
        with open(filepath, 'r', encoding='utf-8') as f:
            for line in f:
                match = local_include_pattern.match(line)
                if match:
                    included_file = os.path.basename(match.group(1))
                    # Only strip if the file is part of our amalgamation
                    # External libraries (like "raylib.h") will be kept intact
                    if included_file in amalgamated_basenames:
                        continue
                processed_lines.append(line)
        return "".join(processed_lines)

    # 2. Setup the STB macros
    guard_macro = generate_macro_name(output_file)
    impl_macro = generate_macro_name(output_file, "_IMPLEMENTATION")

    # 3. Write the final file
    os.makedirs(os.path.dirname(output_file), exist_ok=True)
    
    with open(output_file, 'w', encoding='utf-8') as out:
        # --- HEADER SECTION ---
        out.write(f"/* Automatically generated STB-style header */\n")
        out.write(f"#ifndef {guard_macro}\n")
        out.write(f"#define {guard_macro}\n\n")
        
        for h_file in header_files:
            out.write(f"/* --- Begin {os.path.basename(h_file)} --- */\n")
            out.write(process_file_contents(h_file))
            out.write(f"\n/* --- End {os.path.basename(h_file)} --- */\n\n")
            
        out.write(f"#endif // {guard_macro}\n\n")
        
        # --- IMPLEMENTATION SECTION ---
        out.write(f"/* ==================================================================\n")
        out.write(f"   IMPLEMENTATION SECTION\n")
        out.write(f"   Define {impl_macro} in exactly ONE source file before including.\n")
        out.write(f"   ================================================================== */\n")
        out.write(f"#ifdef {impl_macro}\n\n")
        
        for c_file in source_files:
            out.write(f"/* --- Begin {os.path.basename(c_file)} --- */\n")
            out.write(process_file_contents(c_file))
            out.write(f"\n/* --- End {os.path.basename(c_file)} --- */\n\n")
            
        out.write(f"#endif // {impl_macro}\n")

    print(f"Successfully amalgamated {len(header_files)} headers and {len(source_files)} source files into {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python amalgamate.py <source_directory> <output_file>")
        sys.exit(1)
        
    src_directory = sys.argv[1]
    output_filepath = sys.argv[2]
    
    amalgamate(src_directory, output_filepath)