Here?s a summary table for using LodePNG to load a PNG from a file (given a file path):

| Step | Function/Call           | Description / Parameters                                                                                                     | Output                                                                        |
| ---- | ----------------------- | ---------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| 1    | `lodepng_decode32_file` | Loads PNG and decodes to 32-bit RGBA.<br>Params: `unsigned char **out`, `unsigned *w`, `unsigned *h`, `const char *filename` | `out` = RGBA buffer<br>`w`, `h` = width/height<br>Returns error code (0 = OK) |
| 2    | Check return value      | If return value is 0, decoding succeeded.                                                                                    | Use `out` buffer                                                              |
| 3    | Use image data          | `out` is a heap-allocated buffer of size `w * h * 4` (RGBA).                                                                 | RGBA pixel data                                                               |
| 4    | Free buffer             | `free(out)`                                                                                                                  | Releases memory                                                               |

**Minimal Example:**

```c
#include "lodepng.h"

unsigned char *image;
unsigned width, height;
unsigned error = lodepng_decode32_file(&image, &width, &height, "myimage.png");
if (!error) {
    // use image, width, height
    // image is RGBA, size = width * height * 4
    free(image);
} else {
    // handle error
}
```

**Key Function:**

- `lodepng_decode32_file` is the simplest way to load a PNG from a file path and get RGBA pixel data.

Let me know if you want a more detailed code sample or integration tips for AmigaOS!
