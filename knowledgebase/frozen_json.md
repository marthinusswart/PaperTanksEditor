# Frozen JSON: C/C++ JSON Parser and Emitter

## Features

- ISO C and ISO C++ compliant portable code
- Very small footprint
- No dependencies
- `json_scanf()` scans a string directly into C/C++ variables
- `json_printf()` prints C/C++ variables directly into an output stream
- `json_setf()` modifies an existing JSON string
- `json_fread()` reads JSON from a file
- `json_fprintf()` writes JSON to a file
- Built-in base64 encoder and decoder for binary data
- Parser provides low-level callback API and high-level scanf-like API
- 100% test coverage
- Used in Mongoose OS, an operating system for connected commercial products on low-power microcontrollers

---

## API Reference

### json_scanf(), json_vscanf

```c
int json_scanf(const char *str, int str_len, const char *fmt, ...);
int json_vscanf(const char *str, int str_len, const char *fmt, va_list ap);
```

#### json_scanf's %M handler

```c
typedef void (*json_scanner_t)(const char *str, int len, void *user_data);
```

Scans the JSON string `str`, performing scanf-like conversions according to `fmt`. `fmt` uses scanf()-like format, with the following differences:

- Object keys in the format string don't have to be quoted, e.g. `{key: %d}`
- Order of keys in the format string does not matter, and the format string may omit keys to fetch only those that are of interest.

For example, assume `str` is a JSON string `{ "a": 123, "b": "hi", c: true }`. We can fetch only the value of the `c` key:

```c
int value = 0;
json_scanf(str, strlen(str), "{c: %B}", &value);
```

Several extra format specifiers are supported:

- `%B`: consumes `int` (or `char`, if `sizeof(bool) == sizeof(char)`), expects boolean true or false.
- `%Q`: consumes `char **`, expects quoted, JSON-encoded string. Scanned string is malloc-ed, caller must free() the string.
- `%V`: consumes `char **, int`. Expects base64-encoded string. Result string is base64-decoded, malloced and NUL-terminated. The length of result string is stored in `int` placeholder. Caller must free() the result.
- `%H`: consumes `int, char **`. Expects a hex-encoded string, e.g. "fa014f". Result string is hex-decoded, malloced and NUL-terminated. The length of the result string is stored in `int` placeholder. Caller must free() the result.
- `%M`: consumes custom scanning function pointer and `void *user_data` parameter - see `json_scanner_t` definition.
- `%T`: consumes `struct json_token`, fills it out with matched token.

Returns the number of elements successfully scanned & converted. Negative number means scan error.

#### Example - scan arbitrary JSON string

```c
// str has the following JSON string (notice keys are out of order):
// { "a": 123, "d": true, "b": [1, 2], "c": "hi" }
int a = 0, d = 0;
char *c = NULL;
void *my_data = NULL;
json_scanf(str, strlen(str), "{ a:%d, b:%M, c:%Q, d:%B }",
           &a, scan_array, my_data, &c, &d);

// This function is called by json_scanf() call above.
// str is "[1, 2]", user_data is my_data.
static void scan_array(const char *str, int len, void *user_data) {
  struct json_token t;
  int i;
  printf("Parsing array: %.*s\n", len, str);
  for (i = 0; json_scanf_array_elem(str, len, "", i, &t) > 0; i++) {
    printf("Index %d, token [%.*s]\n", i, t.len, t.ptr);
  }
}
```

#### Example - parse array of objects

```c
// str has the following JSON string - array of objects:
// { "a": [ {"b": 123}, {"b": 345} ] }
// This example shows how to iterate over array, and parse each object.
int i, value, len = strlen(str);
struct json_token t;
for (i = 0; json_scanf_array_elem(str, len, ".a", i, &t) > 0; i++) {
  // t.type == JSON_TYPE_OBJECT
  json_scanf(t.ptr, t.len, "{b: %d}", &value); // value is 123, then 345
}
```

int a = 0, d = 0;
char *c = NULL;
void *my_data = NULL;
json_scanf(str, strlen(str), "{ a:%d, b:%M, c:%Q, d:%B }",
&a, scan_array, my_data, &c, &d);

// This function is called by json_scanf() call above.
\*/

---

### json_scanf_array_elem()

```c
int json_scanf_array_elem(const char *s, int len,
                         const char *path,
                         int index,
                         struct json_token *token);
```

A helper function to scan an array item with given path and index. Fills token with the matched JSON token. Returns -1 if no array element found, otherwise non-negative token length.

---

### json_printf()

Frozen printing API is pluggable. Out of the box, Frozen provides a way to print to a string buffer or to an opened file stream. It is easy to tell Frozen to print to another destination, for example, to a socket, etc. Frozen does this by defining an "output context" descriptor which has a pointer to a low-level printing function. If you want to print to another destination, just define your specific printing function and initialise output context with it.

```c
struct json_out {
  int (*_printer)(struct json_out *, const char *str, size_t len);
  union {
    struct {
      char *buf;
      size_t size;
      size_t len;
    } buf;
    void *data;
    FILE *fp;
  } u;
};

struct json_out out1 = JSON_OUT_BUF(buf, len);
struct json_out out2 = JSON_OUT_FILE(fp);
typedef int (*_json_printf_callback_t)(struct json_out *, va_list *ap);
int json_printf(struct json_out *, const char *fmt, ...);
int json_vprintf(struct json_out *, const char *fmt, va_list ap);
```

Generate formatted output into a given string buffer, auto-escaping keys. This is a superset of printf() function, with extra format specifiers:

- `%B` print json boolean, true or false. Accepts an int.
- `%Q` print quoted escaped string or null. Accepts a const char \*.
- `%.*Q` same as %Q, but with length. Accepts int, const char \*
- `%V` print quoted base64-encoded string. Accepts a const char \*, int.
- `%H` print quoted hex-encoded string. Accepts a int, const char \*.
- `%M` invokes a json_printf_callback_t function. That callback function can consume more parameters.

Return number of bytes printed. If the return value is bigger than the supplied buffer, that is an indicator of overflow. In the overflow case, overflown bytes are not printed.

#### Example

```c
json_printf(&out, "{%Q: %d, x: [%B, %B], y: %Q}", "foo", 123, 0, -1, "hi");
// Result:
// {"foo": 123, "x": [false, true], "y": "hi"}
```

To print a complex object (for example, serialise a structure into an object), use %M format specifier:

```c
struct my_struct { int a, b; } mys = {1,2};
json_printf(&out, "{foo: %M, bar: %d}", print_my_struct, &mys, 3);
// Result:
// {"foo": {"a": 1, "b": 2}, "bar": 3}
int print_my_struct(struct json_out *out, va_list *ap) {
  struct my_struct *p = va_arg(*ap, struct my_struct *);
  return json_printf(out, "{a: %d, b: %d}", p->a, p->b);
}
```

---

### json_printf_array()

```c
int json_printf_array(struct json_out *, va_list *ap);
```

A helper %M callback that prints contiguous C arrays. Consumes void *array_ptr, size_t array_size, size_t elem_size, char *fmt. Returns number of bytes printed.

---

### json_walk() - Low Level Parsing API

```c
// JSON token type
enum json_token_type {
  JSON_TYPE_INVALID = 0, // memsetting to 0 should create INVALID value
  JSON_TYPE_STRING,
  JSON_TYPE_NUMBER,
  JSON_TYPE_TRUE,
  JSON_TYPE_FALSE,
  JSON_TYPE_NULL,
  JSON_TYPE_OBJECT_START,
  JSON_TYPE_OBJECT_END,
  JSON_TYPE_ARRAY_START,
  JSON_TYPE_ARRAY_END,
  JSON_TYPES_CNT,
};

// Structure containing token type and value. Used in json_walk() and
// json_scanf() with the format specifier %T.
struct json_token {
  const char *ptr; // Points to the beginning of the value
  int len;         // Value length
  enum json_token_type type; // Type of the token
};

// Callback-based API
typedef void (*json_walk_callback_t)(void *callback_data,
                                     const char *name, size_t name_len,
                                     const char *path,
                                     const struct json_token *token);

// Parse json_string, invoking callback in a way similar to SAX parsers
int json_walk(const char *json_string, int json_string_length,
              json_walk_callback_t callback, void *callback_data);
```

json_walk() is a low-level, callback based parsing API. json_walk() calls a given callback function for each scanned value.

Callback receives a name, a path to the value, a JSON token that points to the value and an arbitrary user data pointer.

The path is constructed using this rule:

- Root element has "" (empty string) path
- When an object starts, . (dot) is appended to the path
- When an object key is parsed, a key name is appended to the path
- When an array is parsed, an [ELEMENT_INDEX] is appended for each element

For example, consider the following json string:

```json
{ "foo": 123, "bar": [1, 2, { "baz": true }] }
```

The sequence of callback invocations will be as follows:

```
type: JSON_TYPE_OBJECT_START, name: NULL, path: "", value: NULL
type: JSON_TYPE_NUMBER, name: "foo", path: ".foo", value: "123"
type: JSON_TYPE_ARRAY_START, name: "bar", path: ".bar", value: NULL
type: JSON_TYPE_NUMBER, name: "0", path: ".bar[0]", value: "1"
type: JSON_TYPE_NUMBER, name: "1", path: ".bar[1]", value: "2"
type: JSON_TYPE_OBJECT_START, name: "2", path: ".bar[2]", value: NULL
type: JSON_TYPE_TRUE, name: "baz", path: ".bar[2].baz", value: "true"
type: JSON_TYPE_OBJECT_END, name: NULL, path: ".bar[2]", value: "{ \"baz\": true }"
type: JSON_TYPE_ARRAY_END, name: NULL, path: ".bar", value: "[ 1, 2, { \"baz\": true } ]"
type: JSON_TYPE_OBJECT_END, name: NULL, path: "", value: "{ \"foo\": 123, \"bar\": [ 1, 2, { \"baz\": true } ] }"
```

If top-level element is an array:

```json
[1, { "foo": 2 }]
```

```
type: JSON_TYPE_ARRAY_START, name: NULL, path: "", value: NULL
type: JSON_TYPE_NUMBER, name: "0", path: "[0]", value: "1"
type: JSON_TYPE_OBJECT_START, name: "1", path: "[1]", value: NULL
type: JSON_TYPE_NUMBER, name: "foo", path: "[1].foo", value: "2"
type: JSON_TYPE_OBJECT_END, name: NULL, path: "[1]", value: "{\"foo\": 2}"
type: JSON_TYPE_ARRAY_END, name: NULL, path: "", value: "[1, {"foo": 2}]"
```

If top-level element is a scalar:

```json
true
```

```
type: JSON_TYPE_TRUE, name: NULL, path: "", value: "true"
```

---

### json_walk_args() - Extensible Low Level Parsing API

```c
struct frozen_args {
  json_walk_callback_t callback;
  void *callback_data;
  int limit;
};

struct frozen_args args[1];
INIT_FROZEN_ARGS(args);
args->callback = mycb;
args->callback_data = data;
args->limit = 20;
ret = json_walk_args(string, len, args);
```

The `limit` member of struct frozen_args can be set to limit the maximum recursion depth to prevent possible stack overflows and limit parsing complexity.

---

### json_fprintf(), json_vfprintf()

```c
int json_fprintf(const char *file_name, const char *fmt, ...);
int json_vfprintf(const char *file_name, const char *fmt, va_list ap);
```

### json_asprintf(), json_vasprintf()

```c
char *json_asprintf(const char *fmt, ...);
char *json_vasprintf(const char *fmt, va_list ap);
```

Print JSON into an allocated 0-terminated string. Return allocated string, or NULL on error.

#### Example

```c
char *str = json_asprintf("{a:%H}", 3, "abc");
char *content = json_fread("settings.json");
json_scanf(content, strlen(content), "{a: %d, b: %Q}", &c.a, &c.b);
```

---

### json_fread()

```c
char *json_fread(const char *file_name);
```

Read the whole file in memory. Return malloc-ed file content, or NULL on error. The caller must free().

---

### json_setf(), json_vsetf()

```c
int json_setf(const char *s, int len, struct json_out *out,
              const char *json_path, const char *json_fmt, ...);
int json_vsetf(const char *s, int len, struct json_out *out,
               const char *json_path, const char *json_fmt, va_list ap);
```

Update given JSON string `s,len` by changing the value at given `json_path`. The result is saved to `out`. If `json_fmt` == NULL, that deletes the key. If path is not present, missing keys are added. Array path without an index pushes a value to the end of an array. Return 1 if the string was changed, 0 otherwise.

#### Example

```c
// s is a JSON string { "a": 1, "b": [ 2 ] }
json_setf(s, len, out, ".a", "7");      // { "a": 7, "b": [ 2 ] }
json_setf(s, len, out, ".b", "7");      // { "a": 1, "b": 7 }
json_setf(s, len, out, ".b[]", "7");    // { "a": 1, "b": [ 2,7 ] }
json_setf(s, len, out, ".b", NULL);      // { "a": 1 }
```

---

### json_prettify()

```c
int json_prettify(const char *s, int len, struct json_out *out);
```

Pretty-print JSON string `s,len` into `out`. Return number of processed bytes in `s`.

### json_prettify_file()

```c
int json_prettify_file(const char *file_name);
```

Prettify JSON file `file_name`. Return number of processed bytes, or negative number of error. On error, file content is not modified.

---

### json_next_key(), json_next_elem()

```c
void *json_next_key(const char *s, int len, void *handle, const char *path,
                    struct json_token *key, struct json_token *val);
void *json_next_elem(const char *s, int len, void *handle, const char *path,
                    int *idx, struct json_token *val);
```

Iterate over an object or array at given JSON `path`. On each iteration, fill the `key` and `val` tokens (for objects) or array index `idx` (for arrays). Return an opaque value suitable for the next iteration, or NULL when done.

#### Example

```c
void *h = NULL;
struct json_token key, val;
while ((h = json_next_key(s, len, h, ".foo", &key, &val)) != NULL) {
  printf("[%.*s] -> [%.*s]\n", key.len, key.ptr, val.len, val.ptr);
}
```

---

## Minimal Mode

By building with `-DJSON_MINIMAL=1` footprint can be significantly reduced. The following limitations apply in this configuration:

- Only integer numbers are supported. This affects parsing and %f/%lf conversions in printf and scanf.
- Hex ('%H') and base64 (%V) conversions are disabled.

---

## Examples

### Print JSON configuration to a file

```c
json_fprintf("settings.json", "{ a: %d, b: %Q }", 123, "string_value");
json_prettify_file("settings.json"); // Optional
```

### Read JSON configuration from a file

```c
struct my_config { int a; char *b; } c = { .a = 0, .b = NULL };
char *content = json_fread("settings.json");
json_scanf(content, strlen(content), "{a: %d, b: %Q}", &c.a, &c.b);
```

### Modify configuration setting in a JSON file

```c
const char *settings_file_name = "settings.json", *tmp_file_name = "tmp.json";
char *content = json_fread(settings_file_name);
FILE *fp = fopen(tmp_file_name, "w");
struct json_out out = JSON_OUT_FILE(fp);
json_setf(content, strlen(content), &out, ".b", "%Q", "new_string_value");
Modify configuration setting in a JSON file
json_prettify_file(tmp_file_name); // Optional
rename(tmp_file_name, settings_file_name);
```

---

## Contributions

To submit contributions, sign Cesanta CLA and send GitHub pull request.

---

## Licensing

Frozen is released under the Apache 2.0 license.

For commercial support and professional services, contact us.

---

## See Also

- **Mongoose Web Server Library** - a robust, open-source solution licensed under GPLv2, designed to seamlessly integrate web server functionality into your embedded devices.
- **Mongoose Wizard** - a no-code visual tool that enables rapid WebUI creation without the need for frontend expertise.
  const char *settings_file_name = "settings.json", *tmp_file_name = "tmp.json";
  char *content = json_fread(settings_file_name);
  FILE *fp = fopen(tmp_file_name, "w");
  struct json_out out = JSON_OUT_FILE(fp);
  json_setf(content, strlen(content), &out, ".b", "%Q", "new_string_value");
  fclose(fp);
  json_prettify_file(tmp_file_name); // Optional
  rename(tmp_file_name, settings_file_name);
  Contributions
  To submit contributions, sign Cesanta CLA and send GitHub pull request.

Licensing
Frozen is released under the Apache 2.0 license.

For commercial support and professional services, contact us.

See also
Mongoose Web Server Library - a robust, open-source solution licensed under GPLv2, designed to seamlessly integrate web server functionality into your embedded devices.
With complementary Mongoose Wizard - a no-code visual tool that enables rapid WebUI creation without the need for frontend expertise.
