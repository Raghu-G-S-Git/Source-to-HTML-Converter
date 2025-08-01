# 🌈 Source to HTML Converter 🚀

Welcome to the **Source to HTML Converter**, a dynamic C-based tool that transforms raw source code into beautifully formatted, syntax-highlighted HTML files! 🎨 Perfect for code reviews, documentation, presentations, or educational content, this project makes your C code shine in any web browser. 🌐

---

## 🌟 Project Overview

The Source to HTML Converter reads C source files (or similar languages) and generates HTML files with syntax highlighting, preserving indentation and properly rendering special characters. It uses a state-machine-based parser to identify code elements like keywords, comments, strings, and more, wrapping them in styled HTML tags for a polished, web-friendly output. 📝

---

## ✨ Key Features

- **🎨 Syntax Highlighting**: Color-codes reserved keywords, comments, strings, numeric constants, and more.
- **📄 HTML Generation**: Converts source code into clean, standards-compliant HTML.
- **🛠️ Modular Parsing**: Uses a state-machine parser to accurately identify code elements.
- **🎨 Customizable Styling**: Includes a CSS file for easy style customization.
- **🔍 Supports Multiple Elements**: Handles preprocessor directives, header files, comments, strings, and ASCII characters.
- **🚀 Fast and Lightweight**: Efficiently processes files with minimal overhead.

---

## 🛠️ Project Structure

The project is organized into modular C files and a CSS stylesheet, each serving a distinct purpose:

- **`s2html_main.c`**: The driver program, handling file I/O and orchestrating the conversion process.
- **`s2html_event.h`**: Defines parser event types, structures, and function prototypes.
- **`s2html_event.c`**: Implements the state-machine parser for identifying code elements.
- **`s2html_conv.h`**: Declares HTML conversion functions.
- **`s2html_conv.c`**: Converts parsed events into HTML with appropriate tags.
- **`styles.css`**: Provides CSS styles for syntax highlighting.
- **`tags`**: A ctags file for code navigation (optional, generated for development).

---

## 🚀 Getting Started

### Prerequisites
- A C compiler (e.g., `gcc`)
- Standard C libraries (`stdio.h`, `string.h`, `ctype.h`)

### Installation
1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/source-to-html.git
   cd source-to-html
   ```
2. Compile the project:
   ```bash
   gcc -o s2html s2html_main.c s2html_event.c s2html_conv.c
   ```
3. Run the converter:
   ```bash
   ./s2html input.c
   ```

---

## 🎮 How to Use

1. **Launch the Program**:
   Run with a source file (e.g., `input.c`) and optionally specify an output file name:
   ```bash
   ./s2html input.c [output]
   ```
   If no output file is specified, it generates `input.c.html`.

2. **View the Output**:
   Open the generated `.html` file (e.g., `input.c.html`) in a web browser to see the syntax-highlighted code.

3. **Example Command**:
   ```bash
   ./s2html test.c
   ```
   **Output**:
   ```
   Output file test.c.html generated
   ```

4. **Customize Styles**:
   Edit `styles.css` to tweak colors or add new styles for different code elements.

---

## 📂 Code Parsing and HTML Output

- **Parser States**: The parser uses a state machine (`PSTATE_IDLE`, `PSTATE_PREPROCESSOR_DIRECTIVE`, etc.) to identify code elements like:
  - Preprocessor directives (`#include`, `#define`)
  - Reserved keywords (`int`, `for`, `struct`, etc.)
  - Comments (single-line `//` and multi-line `/* */`)
  - Strings (`"hello"`) and ASCII characters (`'a'`)
  - Header files (`<stdio.h>`, `"custom.h"`)
  - Numeric constants (`123`, `3.14`)
- **HTML Structure**:
  - Wraps the code in `<pre>` tags for preserved formatting.
  - Uses `<span>` tags with classes (e.g., `reserved_key1`, `comment`) for styling.
  - Links to `styles.css` for visual enhancements.
- **CSS Styling**:
  - `reserved_key1`: Data type keywords (e.g., `int`, `char`) in blue.
  - `reserved_key2`: Control keywords (e.g., `for`, `if`) in goldenrod.
  - `comment`: Green for comments.
  - `preprocess_dir`: Purple for preprocessor directives.
  - `header_file`: Red for header files.
  - `string`: Magenta for strings.
  - `numeric_constant`: Brown for numbers.
  - `ascii_char`: Firebrick for ASCII characters.

---

## 🎉 Example in Action

**Input File (`test.c`)**:
```c
#include <stdio.h>
#define ADD(a,b) (a+b+10)
int main() {
    int x = 42;
    char *str = "hello";
    // This is a comment
    printf("Hi\n");
}
```

**Run the Program**:
```bash
./s2html test.c
```

**Output File (`test.c.html`)**:
```html
<!DOCTYPE html>
<html lang="en-US">
<head>
<title>sode2html</title>
<meta charset="UTF-8">
<link rel="stylesheet" href="styles.css">
</head>
<body style="background-color:rgb(169, 222, 255);">
<pre>
<span class="preprocess_dir">#include &lt;stdio.h&gt;</span>
<span class="preprocess_dir">#define ADD(a,b) (a+b+10)</span>
<span class="reserved_key1">int</span> main() {
    <span class="reserved_key1">int</span> x = <span class="numeric_constant">42</span>;
    <span class="reserved_key1">char</span> *str = <span class="string">"hello"</span>;
    <span class="comment">// This is a comment</span>
    printf(<span class="string">"Hi\n"</span>);
}
</pre>
</body>
</html>
```

**Browser View**:
- Keywords like `int` and `char` in blue.
- `42` in brown.
- `"hello"` in magenta.
- `// This is a comment` in green.
- `#include <stdio.h>` in purple.

---

## 💡 Tips & Notes
- **Input Files**: Supports C source files (`.c`) or similar text-based code files.
- **Output File**: Generated HTML files require `styles.css` in the same directory for styling.
- **Error Handling**: Checks for valid input files and reports errors if files can't be opened or created.
- **Debug Mode**: Enable `#define DEBUG` in `s2html_event.c` or `s2html_main.c` for parsing insights.
- **Extensibility**: The `type` parameter in `html_begin` and `html_end` allows future customization of HTML tags.

---

## 🤝 Contributing

We’d love your help to enhance this project! 🎉
1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/awesome-feature`).
3. Commit your changes (`git commit -m 'Add awesome feature'`).
4. Push to the branch (`git push origin feature/awesome-feature`).
5. Open a pull request.

Report bugs or suggest improvements via GitHub Issues! 🐛

---

## 📜 License

This project is licensed under the MIT License. See the `LICENSE` file for details.

---

## 🌈 Why This Project?

The Source to HTML Converter is a bridge between raw code and stunning web presentations! Whether you're showcasing your code in a portfolio, creating tutorials, or simplifying code reviews, this tool makes your code pop with vibrant syntax highlighting. Dive in and turn your source code into a web masterpiece! 🚀