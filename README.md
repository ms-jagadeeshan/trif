# trif

A command line utitlity for linux

## Functions
- Printing directories in tree format
- Syncing directories
- Printing difference between directories
- Removing duplicates

# Usage: trif [OPTION] .....  [DIRECTORY] .....

| Option  | Description|
|---|---|
| -d, --directory-only  | List directories only.  |
| -f, --full-path  | Print the full path prefix for each file.  |
| -P  --pattern=SOMETEXT  | List only those files that match the pattern given.  |
| -D, --difference  | Print the difference between folders.  |
|  -s, --sync | Sync the folders.  |
|  -L,--level [+ve integer] |  Descend only level directories deep. |
|  -r,--remove-duplicate  | Move the duplicate files to trash   |
|  --file-type=FILETYPE |  List only files of given file type |
| --help | Prints help message |

