import os
import sys
import subprocess
from pathlib import Path

def convert_to_webp(input_folder):
    # Setup paths
    input_path = Path(input_folder).resolve()
    # Assuming script is in Presentations/ and repo root is one level up
    script_dir = Path(__file__).parent.resolve()
    repo_root = script_dir.parent
    output_path = repo_root / 'images'

    if not input_path.is_dir():
        print(f"Error: The input folder '{input_folder}' does not exist or is not a directory.")
        sys.exit(1)

    # Create output directory if it doesn't exist
    output_path.mkdir(parents=True, exist_ok=True)

    # Supported image extensions
    image_extensions = {'.jpg', '.jpeg', '.png', '.bmp', '.tiff'}

    # Process files
    converted_count = 0
    for file_path in input_path.iterdir():
        if file_path.is_file() and file_path.suffix.lower() in image_extensions:
            output_file = output_path / f"{file_path.stem}.webp"

            # cwebp command with quality 80
            cmd = ['cwebp', '-q', '80', str(file_path), '-o', str(output_file)]

            try:
                # Run the cwebp command
                subprocess.run(cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
                print(f"Converted: {file_path.name} -> {output_file.name}")
                converted_count += 1
            except subprocess.CalledProcessError as e:
                print(f"Failed to convert {file_path.name}: {e.stderr.decode('utf-8')}")
            except FileNotFoundError:
                print("Error: 'cwebp' command not found. Please install WebP tools.")
                sys.exit(1)

    print(f"\nFinished. Converted {converted_count} images to WebP.")
    print(f"Output directory: {output_path}")

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python convert_to_webp.py <input_folder>")
        sys.exit(1)

    convert_to_webp(sys.argv[1])
