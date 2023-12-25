# BashGuiHelper
Set of minimalist GUI `(FLTK)` tools written in **`C++`** to be used by `bash` to:
- Browse files
- Enter qualified data, such as IP, numeric, strings, etcetera
- Make list selections
- Make tree selections

## Usage
Launch at the bash command line, note, ***title*** parameter is universal;
### List Selection
Set ***type*** to `"selection"`, and set ***items*** to a tab-separated value.

  `BashGuiHelper --title="long title" --type=selection --items="Item 1\tItem 2\tItem 3 and a half"`

### Choice Selection
Set ***type*** to `"choice"`, and set ***items*** to a tab-separated value.  NOTE: Maximum 3 items.

  `BashGuiHelper --title="long title" --type=choice --prompt="File Exists" --items="Overwrite\tSave As\tCancel"`

### Text Input
Set ***type*** to `"input"`, set `--prompt` and `--default`

  `BashGuiHelper --type=input --prompt="Enter desired text data" --default="default answer"`

### Password
Set ***type*** to `"password"`, set `--prompt`

  `BashGuiHelper --type=password --prompt="Enter secret info"`

### Message
Set ***type*** to `"message"`, set `--message`

  `BashGuiHelper --type=message --message="We will continue with your script, tomorrow, the world"`

### Tree Browser
Set ***type*** to `"tree"`, and set the ***xml*** to an XML file.  An XML representation of the results/selected nodes will be sent to STDOUT.
  
  `BashGuiHelper --title="long title" --type=tree --xml=selection.xml`

### File Browser
Set ***type*** to `"filebrowser"`, specify starting ***directory***, and ***file_type*** to select one file.
  
  `BashGuiHelper --title="long title" --type=filebrowser --directory=$HOME/src --file_type="Image Files (*.png *.jpg *.bmp)"`

### Multi-File Browser
Set ***type*** to `"multifiles"`, specify starting ***directory***, and ***file_type*** to select multiple files.
  
  `BashGuiHelper --title="long title" --type=multifiles --directory=$HOME/src --file_type="Image Files (*.png *.jpg *.bmp)"`

### Directory Browser
Set ***type*** to `"dirbrowser"`, and specify starting ***directory***.
  
  `BashGuiHelper --title="long title" --type=dirbrowser --directory=$HOME/src`

Results are printed to stdout, error returns 1 and sends error information to stderr

## `LibXML2` Class Library
In order to allow tree browsing, I use `XML/XPath` as the tree DB. `xmllint` is already a great XML tool for `bash`.

In addition to the DOM document and node classes, the most useful component of the class library is the `XPathOBJ` class:
- Provide a simple means to resolve XPath queries
- Handles errors for both XML Parsing and XPath queries
- Has type-specific methods (*with specific error handling*) for:
  - Integers
    - NOTE: Integer cast of the floating point results
  - Floating point
    - NOTE: Will also convert numeric strings from XPath `string()` function (*user ***should*** use XPath `number()` function*)
  - Boolean
    - NOTE: Will also convert numeric values to boolean
  - String
  - Node sets

## Building
Use the Makefile

Or, open the folder with VS Code and hit `F5`

## Status
Not yet usable, development emphasis is good architecture
