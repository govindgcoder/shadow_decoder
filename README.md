# shadow_decoder

A C CLI tool developed to reconstruct a `.bmp` image completely in RAM from a dynamic Base64 stream, then parse its headers to extract a hidden steganographic payload.

> A small mini-project i did to practice bit shifting and memory management in C :)

---

## Code Breakdown ( 🕺)

### 1. Dynamic I/O Stream Buffer (`acceptUserInput`)

Standard Base64 inputs can be massive, and we don't know the file size beforehand. Instead of hardcoding a massive buffer array, the tool reads continuously from `stdin` until `EOF`.

```c
char* acceptUserInput(char *initial_buffer, size_t *capacity, size_t *length)

```

* **Memory Strategy:** It starts with a base allocation (256 bytes) and monitors memory bounds via `(*length) + 1 >= *capacity`.
* **Safe Reallocation:** When the limit is hit, it doubles the capacity using `realloc`. A temporary pointer (`temp`) prevents memory leaks by ensuring we don't lose the original buffer address if allocation fails mid-stream.
* **Sanitization:** It runs every incoming byte through `isValidChar` to immediately drop whitespace or invalid stream noise before it ever touches memory.

### 2. The Base64 Bit-Unpacking Engine (`decodeB64`)

Base64 processes data in 4-character chunks (representing 6 bits each) to reconstruct 3 bytes of raw binary data (8 bits each).

```c
uint8_t* decodeB64(char *buffer, uint8_t *binary_buffer, size_t *capacity, size_t *binary_length, size_t length)

```

#### The Bit Transfer Mechanics

The tool reads 4 Base64 characters at a time, maps them to their 6-bit numeric values (`0` to `63`), and packs them into a single 32-bit register (`uint32_t packed`).

```
Base64 Chars:     [ Char 1 ]     [ Char 2 ]     [ Char 3 ]     [ Char 4 ]
6-bit Values:       010101         110011         001100         111100
                  
Packed 24-bits:                 01010111 00110011 00111100

```

* **Packing Logic:**
```c
packed = (packed << 6) | b64_val;

```


Every incoming 6-bit value shifts the existing bits left to clear room for the next chunk.
* **Unpacking Logic:**
To pull the raw 8-bit bytes out of the 24-bit window, the code shifts right and masks the value out:

```c
  binary_buffer[idx] = (uint8_t)((packed >> (2 - i) * 8) & 0xFF);

```

* **Byte 1:** Shift right by 16 bits, mask with `0xFF`
* **Byte 2:** Shift right by 8 bits, mask with `0xFF`
* **Byte 3:** Shift right by 0 bits, mask with `0xFF`
* **Padding Handling:** If the stream ends in `=` or `==`, the tool decrements the write loop count (`3 - paddings`) to avoid writing garbage trailing bytes.

### 3. Structural Parsing (`extractPixelDataOffset`)

Once the raw binary buffer is populated in memory, it contains a full BMP structure. To find our payload, we have to look past the file metadata.

```c
#pragma pack(push, 1)
typedef struct BMPHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;
#pragma pack(pop)

```

* **Why `#pragma pack(1)`?** By default, compilers pad structures to align with CPU word boundaries (often 4 or 8 bytes). This would corrupt our mapping. Forcing a 1-byte alignment ensures the struct matches the exact physical layout of the BMP file header format.
* **Validation:** The function copies the initial bytes into the struct and verifies the magic bytes (`0x4D42` -> 'BM'). If true, it returns `header.offset`, which points directly to where the image pixel data starts.

### 4. Payload Extraction (`main`)

The CLI expects a single argument: a manual offset integer.

```c
uint8_t *target = binary_buffer + pixel_data_offset + hidden_offset;

```

It computes the absolute pointer address inside the RAM buffer, safely checks that it hasn't exceeded the total decoded length, and streams out character data using `putchar` until it hits a null terminator (`\0`).

---

## Quick Start

### Build

Compile using any standard C compiler (GCC/Clang):

```bash
gcc -O3 shadow_decode.c -o shadow_decode

```

### Execution

Pipe a base64 encoded string into the binary and supply the hidden target offset as a CLI parameter:

```bash
cat encoded_image.b64 | ./shadow_decode 1024

```

```

```
