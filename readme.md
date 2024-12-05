# RAG System - Retrieval Augmented Generation

## Introduction

The RAG CPP System is a command-line tool designed to perform Retrieval Augmented Generation using OpenAI's GPT-4 model. It allows users to embed text from various file formats, store embeddings in a SQLite database, and generate answers to queries by retrieving relevant information from the embedded data.

The system supports:

- Embedding text from multiple files and directories.
- Handling of Chinese text with proper Unicode support.
- Querying the embedded data to generate answers using GPT-4.
- Monitoring the progress of the embedding process.

## Features

- **Multi-file and Directory Support**: Embed multiple files and directories recursively.
- **Chinese Text Handling**: Full support for Chinese characters and tokenization.
- **Custom Command-Line Parsing**: No external dependencies for argument parsing.
- **Progress Monitoring**: Monitor the embedding process in real-time.
- **Integration with OpenAI GPT-4**: Generate answers using the GPT-4 API.
- **Support for Various File Formats**:
  - Text files (`.txt`)
  - PDF files (`.pdf`)
  - Image files (`.png`, `.jpg`, `.jpeg`, `.bmp`) with OCR support. (Under developing)

## System Requirements

- **Operating System**: Windows (Win32) with Unicode support.
- **Compiler**: Visual Studio with C++17 support.
- **Libraries and Dependencies**:
  - SQLite3
  - cURL
  - Tesseract OCR (with Simplified Chinese language data)
  - MuPDF (for PDF handling)
  - cppjieba (for Chinese text segmentation)
  - nlohmann/json (for JSON parsing)
- **OpenAI API Key**: An API key with access to GPT-4.

## Installation and Setup

### 1. Clone the Repository

```bash
git clone https://github.com/yourusername/ragcpp.git
cd ragcpp
```

### 2. Install Dependencies

Ensure all required libraries are installed and their headers and library files are accessible to your project.

- **SQLite3**: Database management.
- **cURL**: HTTP requests to OpenAI API.
- **Tesseract OCR**: OCR for image files (include `chi_sim` language data).
- **MuPDF**: PDF text extraction on Windows.
- **cppjieba**: Chinese word segmentation.
- **nlohmann/json**: JSON parsing.

### 3. Set Up OpenAI API Key

Set the `OPENAI_API_KEY` environment variable:

```bash
setx OPENAI_API_KEY "your_openai_api_key"
```

Restart your terminal or IDE to ensure the environment variable is recognized.

## Building the Project

### Using Visual Studio

1. **Open the Solution**: Open the `ragcpp.sln` file in Visual Studio.
2. **Configure Project Settings**:
   - **Character Set**: Set to **Use Unicode Character Set**.
   - **C++ Standard**: Ensure C++17 is enabled.
   - **Include Directories**: Add paths to headers of all dependencies.
   - **Library Directories**: Add paths to library files (`.lib`) of all dependencies.
   - **Additional Dependencies**: Link against required libraries in **Linker** -> **Input**.
3. **Build the Project**:
   - Build the solution (`Ctrl+Shift+B`).

## Usage

The RAG System is a command-line tool that accepts various options:

```bash
ragcpp.exe [options]
```

### Command-Line Options

- `-e, --embed FILE_OR_DIR [FILE_OR_DIR...]`  
  Embed files or directories recursively.

- `-d, --delete ID [ID...]`  
  Delete documents by their IDs.

- `-q, --query QUERY`  
  Query the embedded data and generate an answer.

- `-l, --list`  
  List existing documents in the database.

- `-m, --monitor`  
  Monitor the embedding progress in real-time.

- `-h, --help`  
  Display the help message.

### Examples

#### 1. Embedding Multiple Files and Directories

To embed multiple files and directories:

```bash
ragcpp.exe --embed documents\file1.txt images\ image2.jpg
```

This command will:

- Embed `file1.txt` located in the `documents` directory.
- Recursively embed all supported files in the `images` directory.
- Embed the image file `image2.jpg`.

#### 2. Querying the Embedded Data

To generate an answer based on the embedded data:

```bash
ragcpp.exe --query "美國的首都是哪裡？"
```

This command will:

- Use the question "美國的首都是哪裡？" (What is the capital of USA?) to query the database.
- Retrieve relevant information from the embedded data.
- Generate an answer using GPT-4 with proper citations.

#### 3. Deleting Documents

To delete documents by their IDs:

```bash
ragcpp.exe --delete 1 2 3
```

This command will delete documents with IDs 1, 2, and 3 from the database.

#### 4. Listing Documents

To list all existing documents in the database:

```bash
ragcpp.exe --list
```

This command will display all documents with their IDs and file names.

#### 5. Monitoring Embedding Progress

To monitor the progress of the embedding process:

```bash
ragcpp.exe --monitor
```

This command will display real-time updates of the embedding process, showing the progress for each document being embedded.

## Notes on Chinese and Unicode Text Handling

- **Unicode Support**: The application uses `std::wstring` and `wchar_t` to handle Unicode text, ensuring proper processing of Chinese characters.
- **Console Output**: The console code page is set to UTF-8 to correctly display Unicode characters.
- **Encoding Conversions**: Encoding conversion functions are used to convert between UTF-8 and UTF-16 where necessary.
- **Chinese Text Segmentation**: `cppjieba` is used for tokenizing Chinese text before embedding.
- **OCR for Images**: Tesseract OCR with the Simplified Chinese language data (`chi_sim`) is used to extract text from images.

## Contributing

Contributions are welcome! If you have suggestions, bug reports, or improvements, please open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

---

**Disclaimer**: Ensure you comply with OpenAI's API terms and conditions when using their services. This tool is for educational purposes and should be used responsibly.

---

# RAG 系統 - 檢索增強生成

## 介紹

RAG 系統是一個命令列工具，設計用於使用 OpenAI 的 GPT-4 模型執行檢索增強生成。它允許使用者從各種文件格式中嵌入文本，將嵌入存儲在 SQLite 資料庫中，並通過從嵌入的數據中檢索相關資訊來生成查詢的答案。

系統支援：

- 從多個文件和目錄中嵌入文本。
- 正確的 Unicode 支援，用於處理中文文本。
- 查詢嵌入的數據以使用 GPT-4 生成答案。
- 監控嵌入過程的進度。

## 特點

- **多文件和目錄支援**：遞迴嵌入多個文件和目錄。
- **中文文本處理**：全面支援中文字符和斷詞。
- **自訂命令列解析**：無需外部依賴進行參數解析。
- **進度監控**：實時監控嵌入過程。
- **與 OpenAI GPT-4 集成**：使用 GPT-4 API 生成答案。
- **支援多種文件格式**：
  - 文本文件（`.txt`）
  - PDF 文件（`.pdf`）
  - 圖像文件（`.png`、`.jpg`、`.jpeg`、`.bmp`），支援 OCR。

## 系統需求

- **作業系統**：支援 Unicode 的 Windows（Win32）。
- **編譯器**：支援 C++17 的 Visual Studio。
- **庫和依賴**：
  - SQLite3
  - cURL
  - Tesseract OCR（包含簡體中文語言資料）
  - MuPDF（用於處理 PDF）
  - cppjieba（用於中文分詞）
  - nlohmann/json（用於 JSON 解析）
- **OpenAI API 金鑰**：具有 GPT-4 訪問權限的 API 金鑰。

## 安裝和設定

### 1. 克隆倉庫

```bash
git clone https://github.com/yourusername/ragcpp.git
cd ragcpp
```

### 2. 安裝依賴

確保所有必需的庫已安裝，並且其標頭文件和庫文件對您的項目可用。

- **SQLite3**：資料庫管理。
- **cURL**：向 OpenAI API 發送 HTTP 請求。
- **Tesseract OCR**：用於圖像文件的 OCR（包括簡體中文語言資料 `chi_sim`）。
- **MuPDF**：在 Windows 上處理 PDF 文本提取。
- **cppjieba**：中文分詞。
- **nlohmann/json**：JSON 解析。

### 3. 設定 OpenAI API 金鑰

設定 `OPENAI_API_KEY` 環境變數：

```bash
setx OPENAI_API_KEY "your_openai_api_key"
```

重新啟動您的終端或 IDE，確保環境變數被識別。

## 構建項目

### 使用 Visual Studio

1. **打開解決方案**：在 Visual Studio 中打開 `ragcpp.sln` 文件。
2. **配置項目設置**：
   - **字符集**：設置為 **使用 Unicode 字符集**。
   - **C++ 標準**：確保啟用了 C++17。
   - **包含目錄**：添加所有依賴項的標頭文件路徑。
   - **庫目錄**：添加所有依賴項的庫文件（`.lib`）路徑。
   - **附加依賴項**：在 **連結器** -> **輸入** 中連結所需的庫。
3. **構建項目**：
   - 構建解決方案（`Ctrl+Shift+B`）。

## 使用方法

RAG 系統是一個命令列工具，接受各種選項：

```bash
ragcpp.exe [options]
```

### 命令列選項

- `-e, --embed FILE_OR_DIR [FILE_OR_DIR...]`  
  嵌入文件或目錄（可指定多個）。

- `-d, --delete ID [ID...]`  
  根據 ID 刪除文檔。

- `-q, --query QUERY`  
  查詢嵌入的數據並生成答案。

- `-l, --list`  
  列出資料庫中現有的文檔。

- `-m, --monitor`  
  實時監控嵌入進度。

- `-h, --help`  
  顯示幫助信息。

### 示例

#### 1. 嵌入多個文件和目錄

要嵌入多個文件和目錄：

```bash
ragcpp.exe --embed documents\file1.txt images\ image2.jpg
```

此命令將：

- 嵌入位於 `documents` 目錄下的 `file1.txt`。
- 遞迴嵌入 `images` 目錄中的所有支援文件。
- 嵌入圖像文件 `image2.jpg`。

#### 2. 查詢嵌入的數據

要基於嵌入的數據生成答案：

```bash
ragcpp.exe --query "美國的首都是哪裡？"
```

此命令將：

- 使用問題「美國的首都是哪裡？」來查詢資料庫。
- 從嵌入的數據中檢索相關資訊。
- 使用 GPT-4 生成帶有正確引用的答案。

#### 3. 刪除文檔

要根據 ID 刪除文檔：

```bash
ragcpp.exe --delete 1 2 3
```

此命令將從資料庫中刪除 ID 為 1、2 和 3 的文檔。

#### 4. 列出文檔

要列出資料庫中所有現有的文檔：

```bash
ragcpp.exe --list
```

此命令將顯示所有文檔及其 ID 和文件名。

#### 5. 監控嵌入進度

要監控嵌入過程的進度：

```bash
ragcpp.exe --monitor
```

此命令將實時顯示嵌入過程的更新，顯示每個正在嵌入的文檔的進度。

## 關於中文和 Unicode 文本處理的注意事項

- **Unicode 支援**：應用程式使用 `std::wstring` 和 `wchar_t` 來處理 Unicode 文本，確保正確處理中文字符。
- **控制台輸出**：控制台代碼頁設置為 UTF-8，以正確顯示 Unicode 字符。
- **編碼轉換**：在需要時使用編碼轉換函數在 UTF-8 和 UTF-16 之間進行轉換。
- **中文分詞**：使用 `cppjieba` 在嵌入之前對中文文本進行斷詞。
- **圖像的 OCR**：使用包含簡體中文語言資料（`chi_sim`）的 Tesseract OCR 從圖像中提取文本。

## 貢獻

歡迎貢獻！如果您有建議、錯誤報告或改進，請開啟問題或提交拉取請求。

## 授權

此項目採用 [MIT 許可證](LICENSE)。

---

**聲明**：在使用 OpenAI 的服務時，請確保遵守其 API 條款和條件。此工具僅供教育用途，應負責任地使用。

---
