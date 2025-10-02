# OCR-KeltIA


## git hooks

To enable the hooks provided by this repo you will need to configure 
the git config for this repo, the following command will enable it.

```bash
# in the root of the repo
git config core.hooksPath scripts/git-hooks
```

### pre-commit hook

This hook is triggered before commiting any change to git, it
looks into the commited files if there is any .c or .h file and
formats it with clang-format with the following the rules found 
in the file .clang-format.
