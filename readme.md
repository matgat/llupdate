## [llupdate](https://github.com/matgat/llupdate.git)
A tool that updates the external libraries in a LogicLab project file.
The supported project formats:
* `.ppjs` (LogicLab3 project)
* `.plcprj` (LogicLab5 project)



_________________________________________________________________________
## Usage
Windows binary is dynamically linked to Microsoft c++ runtime,
so needs the installation of
[`VC_redist.x64.exe`](https://aka.ms/vs/17/release/vc_redist.x64.exe)
as prerequisite.

To print usage info:

```bat
> llupdate --help
```

To update a project:

```bat
> llupdate "C:\path\to\project.ppjs"
```

To update a project without overwriting the original file:

```bat
> llupdate "C:\path\to\project.ppjs" --out "C:\path\to\project-updated.ppjs"
```

Note that the output file will be overwritten without any warning.

| Return value | Meaning                                |
|--------------|----------------------------------------|
|      0       | Operation successful                   |
|      1       | Operation completed but with issues    |
|      2       | Operation aborted due to a fatal error |



_________________________________________________________________________
## Build

```sh
# pacman -S fmt
$ git clone https://github.com/matgat/llupdate.git
$ cd llupdate
$ make linux/makefile
```

Or directly:

```sh
$ g++ -std=c++2b -Wall -Wextra -Wpedantic -Wconversion -O3 -lfmt -o "linux/build/llupdate" "source/main.cpp"
```

Testing:

```sh
$ g++ -std=c++2b -Wall -Wextra -Wpedantic -Wconversion -O3 -lfmt -o "linux/build/llupdate_test" "test/test.cpp" && linux/build/llupdate_test
```

On Windows use the latest Microsoft Visual Studio Community.
From the command line, something like:

```bat
> msbuild .msvc/llupdate.vcxproj -t:Rebuild -p:Configuration=Release -p:Platform=x64
```

This project depends on `{fmt}` library, use `vcpkg` to install it:

```bat
> git clone https://github.com/Microsoft/vcpkg.git
> .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
> .\vcpkg\vcpkg integrate install
> .\vcpkg\vcpkg install fmt:x64-windows
```

in case you already have `vcpkg`:

```bat
> .\vcpkg\bootstrap-vcpkg.bat -disableMetrics
> .\vcpkg\vcpkg upgrade --no-dry-run
```
