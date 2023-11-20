# BashQtHelper
Set of minimalist GUI `(Qt)` tools written in **`C++`** to be used by `bash` to:
- Browse files
- Enter qualified data, such as IP, numeric, strings, etcetera
- Make list selections
- Make tree selections

## Usage
Launch at the bash command line, note, ***title*** parameter is universal;
### List Selection
Set the ***type*** to `"selection"`, and set the ***items*** to a tab-separated value. .
  
  `BashQtHelper --title="long title" --type=selection --items="Item 1\tItem 2\tItem 3 and a half"`

### File Browser
Set the ***type*** to `"filebrowser"`, starting ***directory***, and ***file_type*** to select one file.
  
  `BashQtHelper --title="long title" --type=filebrowser --directory=$HOME/src --file_type="Image Files (*.png *.jpg *.bmp)"`

### Multi-File Browser
Set the ***type*** to `"multifiles"`, starting ***directory***, and ***file_type*** to select multiple files.
  
  `BashQtHelper --title="long title" --type=filebrowser --directory=$HOME/src --file_type="Image Files (*.png *.jpg *.bmp)"`

### Directory Browser
Set the ***type*** to `"multifiles"`, and starting ***directory***.
  
  `BashQtHelper --title="long title" --type=multifiles --directory=$HOME/src`

Results are printed to stdout, error returns 1 and sends error information to stderr

## `LibXML2` Class Library
In order to allow tree browsing, I use `XML/XPath` as the tree DB. `xmllint` is already a great XML tool for `bash`.

In addition to the DOM document and node classes, the most useful component of the class library is the `XPathOBJ` class:
- Provide a simple means to resolve XPath queries
- Handles errors for both XML Parsing and XPath queries
- Has type-specific methods (*with specific error handling*) for:
  - Integers
  - Floating point
    - NOTE: Will convert numeric strings from XPath `string()` function (*user should use `number()` function*)
  - Boolean
    - NOTE: Will convert numeric values to boolean
  - String
  - Node sets

## Building
Use the Makefile

Or, open the folder with VS Code and hit `F5`

## Status
Not yet usable, development emphasis is good architecture
