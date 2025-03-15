# Contributing guide

## Type of contributing

There are two main types of contributions accepted in KolibriOS:

- Submitting issues about problems in the project
- Submitting code to the project via pull requests

Each of these types is described in detail below.

## Issues

You can help us by submitting issues about problems found in the system.
Currently, there are two main ways of submitting an issue in the project:
**Bug Reports** and **Feature Requests**:

- Bug Reports are suitable if you find a **bug** (crash, error, unexpected
behavior) in some part of the system (kernel, drivers, apps, etc.) and want to
report it
- Feature Request are used, when you want to propose some **improvement** to
the system (missing features, improved  user experience, etc.)

## Pull requests

You can also help us by submitting code via pull requests. The process of
submitting a pull request consists of the following steps:

1. Find what you want to implement or improve
2. Make a fork of kolibrios (or other needed) repository
3. Create a branch with a name that matches [the style](#branch-style)
4. Implement and test the changes
5. Create commits according to the [accepted style](#commit-style)
6. Create and submit a pull request into `main` branch
7. Wait for CI/CD pipelines and code review to pass

When a pull request is submitted, at least two project participants must conduct
a code review, after which the proposed changes can be corrected (if it's
necessary) and merged into the project.

## Branch style

1. Your branch name should be as short as possible, but describes your changes
2. Words should be divided by minus sign (`-`)
3. Optionally, might starts with general [type](#types) of your future PR
with slash (`/`): `refactor/nasm-to-fasm`, `update/demos`, `fix/cp866-charset`

## Commit style

### Pattern

The commit message should look like this:

```
type(scope): commit message header

Commit message body, if needed
```
- Use the present tense ("Add feature" not "Added feature")
- Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit the first line to 72 characters or less
- Reference issues and pull requests liberally after the first line
- When only changing documentation, include [ci skip] in the commit title
- Commit message header and body should reflect changes made in commit

### Types

|       Type       | Description                                       |
| :--------------: | :------------------------------------------------ |
| `feat / feature` | for new feature implementing commit               |
|     `update`     | for update commit                                 |
|      `bug`       | for bug fix commit                                |
|    `security`    | for security issue fix commit                     |
|  `performance`   | for performance issue fix commit                  |
|  `improvement`   | for backwards-compatible enhancement commit       |
|    `breaking`    | for backwards-incompatible enhancement commit     |
|   `deprecated`   | for deprecated feature commit                     |
|      `i18n`      | for i18n (internationalization) commit            |
|      `a11y`      | for a11y (accessibility) commit                   |
|    `refactor`    | for refactoring commit                            |
|      `docs`      | for documentation commit                          |
|    `example`     | for example code commit                           |
|      `test`      | for testing commit                                |
|      `deps`      | for dependencies upgrading or downgrading commit  |
|     `config`     | for configuration commit                          |
|     `build`      | for packaging or bundling commit                  |
|    `release`     | for publishing commit                             |
|      `wip`       | for work in progress commit                       |
|     `chore`      | for other operations commit                       |

### Scopes

> [!NOTE]
> Scopes are optional

| Scope  | Description                      |
| :----: | :------------------------------- |
| `krn`  | kernel                           |
| `drv`  | drivers                          |
| `lib`  | libraries                        |
| `app`  | userspace applications           |
| `skin` | skins                            |
| `data` | images, configs, resources, etc. |

> [!NOTE]
> If changes are made to a specific component, the name of the component
> separated by `/` character needs to be specified. For example:
> `app/shell`, `lib/libimg`

## Merge commits

> [!WARNING]
> Merge commits are **prohibited** in the project

## Conclusion

We hope this small instructions will help you to get familiar  with KolibriOS
contribution rules and inspire you to participate in the work of this project.
