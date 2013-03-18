
Contributing to the boostcache project
=====================================

Codding Style
===============

- https://github.com/azat/azat.github.com/wiki/Cpp-Coding-Style
- https://github.com/azat/azat.github.com/wiki/C-Coding-Style

Files, folders and namespaces
=============================

- You must wrap into namespace every top-level folder, except "kernel" for now

Git Settings
============

    core
        git config core.autocrlf input
        git config core.whitespace trailing-space

    push
        git config push.default upstream

    alias
        git config alias.di diff
        git config alias.ci commit
        git config alias.cis "commit --signoff"
        git config alias.co checkout
        git config alias.ann blame
        git config alias.cst status

    hooks
        install pre-commit hook from utils/git-hooks/client/pre-commit
