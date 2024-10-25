

## Setup clang-format as pre-commit

Add the following script to .git/hooks/pre-commit
https://ortogonal.github.io/cpp/git-clang-format/
```bash
#!/bin/sh  
  
if git rev-parse --verify HEAD >/dev/null 2>&1  
then  
against=HEAD  
else  
# Initial commit: diff against an empty tree object  
against=4b825dc642cb6eb9a060e54bf8d69288fbee4904  
fi  
  
# Test clang-format  
clangformatout=$(git clang-format --diff --staged -q)  
  
# Redirect output to stderr.  
exec 1>&2  
  
if [ "$clangformatout" != "" ]  
then
    echo "Format error!"
    echo "Use git clang-format"
    exit 1
fi
```

This will check for formatting errors for staged changes.
