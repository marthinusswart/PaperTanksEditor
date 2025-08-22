# Next Tasks for PaperTanksEditor

| Task                                            | Description                                                                                                                    |
| ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Palette read from lodepng                       | Implement code to extract full palette from indexed PNGs using lodepng and handle cases for truecolor PNGs.                    |
| On-the-fly palette remapping with libimagequant | Integrate libimagequant for in-memory quantization and remapping of RGBA images to a 256-color palette.                        |
| ImageMagick palette creation                    | Use ImageMagick to generate a 256-color palette by combining a custom 8-color palette with 248 colors extracted from an image. |
| Amiga palette integration                       | Ensure palette and indexed image data are compatible with Amiga AGA hardware and graphics routines.                            |
| Automated color count                           | Add logic to count unique colors in loaded images for diagnostics and palette optimization.                                    |
| Documentation update                            | Expand documentation to cover palette handling, remapping, and external tool workflows.                                        |

# ImageMagick Palette Creation Flow

| Step | Command/Action                | Purpose                                                                     |
| ---- | ----------------------------- | --------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| 1    | Extract 248 colors from image | `magick input.png -colors 248 -unique-colors extracted.png`                 | Get 248 most representative colors from the source image.                                        |
| 2    | Create 8-color palette image  | Manually create `my8.png` with your 8 desired colors as the first 8 pixels. | Define custom system colors.                                                                     |
| 3    | Merge palettes                | `magick +append my8.png extracted.png final_palette.png`                    | Combine the 8 custom colors with the 248 extracted colors into a single 256-color palette image. |
| 4    | Remap image to new palette    | `magick input.png -remap final_palette.png output.png`                      | Apply the combined palette to the image, ensuring the first 8 colors are your custom choices.    |

# Environment Details

Docker Container using Amiga 68k and VBCC
Amiga OS 3.1
Magic User Interface (MUI) 3.8
NDK 3.2
Debugging on a Amiga Emulator
Log file has all the logs

# RGBA vs BGRA

This does not matter at code level, code as if values are RGBA, the gfx driver takes care of the swapping.
