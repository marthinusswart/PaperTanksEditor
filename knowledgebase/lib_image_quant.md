**libimagequant size:**

- The core library is lightweight: the compiled static library (`libimagequant.a`) is typically under 200 KB.
- The source code is a few C files, easy to integrate into Amiga or embedded projects.

---

**libimagequant usage flow summary**

| Step                 | Function(s) / Action                                                | Purpose/Notes                                                 |
| -------------------- | ------------------------------------------------------------------- | ------------------------------------------------------------- |
| 1. Create attributes | `liq_attr *liq_attr_create()`                                       | Initialize quantization attributes                            |
| 2. Prepare image     | `liq_image *liq_image_create_rgba()`                                | Create image object from RGBA byte array                      |
| 3. Set palette       | (Optional, advanced)                                                | You can let libimagequant choose palette, or provide your own |
| 4. Quantize          | `liq_result *liq_quantize_image()`                                  | Quantize image to best-fit palette                            |
| 5. Get palette       | `liq_get_palette()`                                                 | Retrieve generated palette (if you let libimagequant choose)  |
| 6. Remap pixels      | `liq_write_remapped_image()`                                        | Get indexed pixel array (each byte is palette index)          |
| 7. Cleanup           | `liq_result_destroy()`, `liq_image_destroy()`, `liq_attr_destroy()` | Free resources                                                |

---

**If you already have a palette and want to remap:**

| Step                 | Function(s) / Action                        | Purpose/Notes                             |
| -------------------- | ------------------------------------------- | ----------------------------------------- |
| 1. Create attributes | `liq_attr_create()`                         | Initialize attributes                     |
| 2. Prepare image     | `liq_image_create_rgba()`                   | Create image object from RGBA byte array  |
| 3. Set palette       | `liq_set_palette()`                         | Set your own palette (array of colors)    |
| 4. Remap pixels      | `liq_write_remapped_image()`                | Get indexed pixel array (palette indices) |
| 5. Cleanup           | `liq_image_destroy()`, `liq_attr_destroy()` | Free resources                            |

---

**Summary:**

- libimagequant is small and efficient.
- You create an image object, set or generate a palette, and remap pixels to palette indices.
- All operations are in memory; no need to write PNG files unless you want to.

That?s a good approach?libimagequant is designed to be easily embedded. You can simply add its `.c` and `.h` files directly to your project and compile them with your sources.

**How to do it:**

1. Download the libimagequant source (https://github.com/ImageOptim/libimagequant).
2. Copy `libimagequant.c` and `libimagequant.h` into your project?s source directory.
3. Include `libimagequant.h` in your code where needed.
4. Compile `libimagequant.c` along with your other sources.

**Benefits:**

- No need for external libraries or linking.
- Easy integration and portability.
- You can use all libimagequant features in-memory.

**Summary:**  
Just add `libimagequant.c` and `libimagequant.h` to your project, include the header, and compile as usual. This is a common and supported usage pattern for libimagequant.
