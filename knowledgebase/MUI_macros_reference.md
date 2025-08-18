# MUI Macros and Usage Reference

This document summarizes the most important macros and helpers from the Amiga MUI (Magic User Interface) `mui.h` header, as found in your SDK. Use this as a quick reference for available macros and their usage in your C projects.

---

## Object Creation Macros

- `ApplicationObject`, `WindowObject`, `VGroup`, `HGroup`, `ColGroup(cols)`, `RowGroup(rows)`, `PageGroup`, etc.
- `StringObject`, `TextObject`, `RectangleObject`, `ListObject`, `ListviewObject`, `CycleObject`, `RadioObject`, `SliderObject`, `ButtonObject`, etc.
- Each macro expands to `MUI_NewObject` with the appropriate class and tags.

### Example
```c
app = ApplicationObject,
    SubWindow, window = WindowObject,
        WindowContents, VGroup,
            Child, StringObject, ... End,
        End,
    End,
End;
```

---

## Label Macros

- `Label(label)` — Simple label, no frame
- `Label1(label)` — Label for single-frame objects (e.g., checkmarks)
- `Label2(label)` — Label for double-frame objects (e.g., string gadgets)
- `LLabel(label)` — Left-aligned label
- `CLabel(label)` — Centered label
- `FreeLabel(label)` — Vertically free label
- ...and combinations (see mui.h for all variants)

### Example
```c
Child, Label("Name"),
Child, Label1("Enabled"),
Child, LLabel("Left Aligned"),
```

---

## Framing Macros

- `NoFrame`, `ButtonFrame`, `TextFrame`, `StringFrame`, `GroupFrame`, `GroupFrameT(title)`, etc.
- Use as tag pairs: `MUIA_Frame, MUIV_Frame_*`

### Example
```c
Child, RectangleObject, StringFrame, End,
Child, VGroup, GroupFrameT("Section Title"), ... End,
```

---

## Spacing Macros

- `HSpace(x)`, `VSpace(x)`, `HVSpace`, `RectSpacer(width, height)`
- `GroupSpacing(x)`, `InnerSpacing(h, v)`

---

## String Macros

- `String(contents, maxlen)` — (Obsolete, but available)
- `TxtInput(ftxt)` — Custom macro in your project for string input fields

---

## Checkmark Macros

- `CheckMark(selected)` — Standard checkmark
- `KeyCheckMark(selected, control)` — Checkmark with keyboard shortcut

---

## Button Macros

- `SimpleButton(label)` — Simple button
- `KeyButton(name, key)` — Button with keyboard shortcut

---

## Cycle/Radio/Slider Macros

- `Cycle(entries)`, `Radio(name, array)`, `Slider(min, max, level)`

---

## Child/Window Macros

- `Child` — Used to add a child object to a group
- `SubWindow` — Used to add a window to an application
- `WindowContents` — Used to set the root object of a window

---

## Set/Get Helpers

- `set(obj, attr, value)` — Set attribute
- `get(obj, attr, store)` — Get attribute
- `nnset(obj, attr, value)` — Set attribute without notification

---

## Useful Label Flags

- `MUIO_Label_SingleFrame`, `MUIO_Label_DoubleFrame`, `MUIO_Label_LeftAligned`, `MUIO_Label_Centered`, `MUIO_Label_FreeVert`, etc.

---

## Example: Left-Aligned Label
```c
Child, LLabel("My Label"),
```

---

## For More
See the full `mui.h` for all available macros, object types, and attribute definitions.
