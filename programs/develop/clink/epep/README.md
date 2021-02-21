# epep - Embeddable PE Parser
## Features

- PE header (including Data Directories as a part Optional Header)
- Section Headers
- COFF Symbols
- Imports
- Exports
- Base relocations (DLL)

## How to use

To declare functions from the library include it:

```C
#include "epep.h"
```

The functions they shoud be instantiated somewhere in the project like so:

```C
#define EPEP_INST
#include "epep.h"
```
