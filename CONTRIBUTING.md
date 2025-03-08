# Contributing guide

The __KolibriOS__ project __Contributing__ guide

## Overview

The process of contributing to the project is described in detail in this document.

## Code of Conduct

The first thing to do is to read and follow our [Code of Conduct](./CODE_OF_CONDUCT.md).

## Type of contributing

There are two main types of contributions: submitting issues about problems and submitting pull requests. Each of these types of contributions is described in detail below.

## Issues

You can help us by submitting issues to repository. At the moment there are 2 main ways of submitting an issue in the project: _bug reports_ and _feature requests_. All existing forms are provided with brief explanations

- Reports are suitable if you find a __bug__ in some part of the project and want to create a report;
- Also there is a possibility submit a __feature request__ if it seems to you that the project lacks any feature.

## Pull requests

You can also help us by submitting pull requests. The process of submitting a pull request consists of several steps:

- Find what you want to implement or improve;
- Make a fork (not relevant for owners);
- Create a branch with a name that matches your changes;
- Make and test the changes;
- Create commits according to the [accepted style](##commit-style);
- Create and submit a pull request;
- Wait for CI/CD pipelines and code review to pass.

When a pull request is submitted, at least two project participants must conduct a code review, after which the changes are corrected (if it's necessary) and merged into the project.

## Commit style

### Naming

- Pattern

  Regular commit message should consist of several parts and be built according to the following template:

  ```test
  Category: Commit message body

  Long description if necessary
  ```

  - Commit message body and description should briefly reflect the meaning of the commit changes;
  - Commit message body should be written starting with a capital letter;
  - Description should be separated from the body by one empty line.

- Length

  The maximum number of characters in a commit header is __72__ (standard for __Git__). Additionally, __72__ is the maximum length of a line in a commit body.

- Categories

  List of existing categories accepted in the project:

  - `Krn` - kernel
  - `Drv` - drivers
  - `Libs` - libraries
  - `Skins` - skins
  - `Build` - build system
  - `CI/CD` - CI/CD
  - `Docs` - documentation
  - `Data` - images, configs, resources, etc.

  List of categories for exceptional situations:

  - `All` - global changes
  - `Other` - unclassifiable changes

  If changes are made to a specific component, the name of the component separated by `/` character needs to be specified. For example:

  ```text
  Apps/shell
  Libs/libimg
  ```

### Merge commits

Merge commits are __prohibited__ in the project

### Unwanted commits

Commit messages like `Refactoring`/`Update files` are __unwanted__ in the project

### Signing

This is not a requirement, but it is strongly recommended to sign commits. This can be done by the following command:

```sh
git commit -s
```
