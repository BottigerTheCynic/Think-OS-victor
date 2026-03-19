# ThinkOS - /anki folder

**This is where the personality code for ThinkOS exists.**

For the entire OS, check out the [think-os](https://github.com/BottigerTheCynic/Think-OS) repo. This repo, `Think-OS-victor`, is a submodule of that, and just builds the `/anki` folder which goes into that OS. However, `Think-OS-victor` can still be built standalone and deployed to a robot which is running a good base OTA. This is recommended for developers.

Most changes happen in this repo. If one wants to, for instance, add a new feature — this is where they’d do it.

If you want to add a program to the OS, do that in [think-os](https://github.com/BottigerTheCynic/Think-OS).

Check the [ThinkOS docs](https://github.com/BottigerTheCynic/Think-OS/wiki) for more information about the source code, what we can do with this, and general Vector info.

-----

## What is ThinkOS?

ThinkOS serves as a productivity-focused, stable, and easily-buildable custom firmware for Vector. It is designed to help you stay productive, focused, and organized — with Vector as your personal productivity companion.

Any feature added here should be productivity-focused or otherwise generally useful. Feel free to make a PR. ThinkOS encourages PRs which add things like:

- New productivity behaviors
- 3rd-party library upgrades
- Code documentation
- Optimizations

-----

## Building

`Think-OS-victor` can be built standalone on most Linux distros (arm64 or amd64) and on macOS (arm64/M-series).

For Linux, the Docker method is recommended for now (especially if you have a weird or old Linux distro installed), though bare metal works nicely too.

> **Note:** If you have built in Docker before and want to build on bare metal now (or vice-versa), you should do a [clean](#cleaning) build first.

Click an option below for instructions.

<details>
<summary><strong>🐳 Docker: x86_64 or arm64 Linux</strong></summary>
<br />

**Prerequisites:** Make sure you have `docker` and `git` installed.

1. Clone the repo and `cd` into it:

```bash
cd ~
git clone --recurse-submodules https://github.com/BottigerTheCynic/Think-OS-victor
cd Think-OS-victor
```

1. Make sure you can run Docker as a normal user:

```bash
sudo groupadd docker
sudo gpasswd -a $USER docker
newgrp docker
sudo chown root:docker /var/run/docker.sock
sudo chmod 660 /var/run/docker.sock
```

1. Run the build script:

```bash
cd ~/Think-OS-victor
./build/build-v.sh
```

</details>

<details>
<summary><strong>🖥️ Bare Metal: x86_64 or arm64 Linux</strong></summary>
<br />

**Prerequisites:**

- glibc 2.35 or above (anything Debian Bookworm-era and newer will work)
- The following packages must be installed: `git wget curl openssl ninja g++ gcc pkg-config ccache`

```bash
# Arch Linux:
sudo pacman -S git wget curl openssl ninja gcc pkgconf ccache

# Ubuntu/Debian:
sudo apt-get update && sudo apt-get install -y git wget curl openssl ninja-build gcc g++ pkg-config ccache

# Fedora:
sudo dnf install -y git wget curl openssl ninja-build gcc gcc-c++ pkgconf-pkg-config ccache
```

1. Clone the repo and `cd` into it:

```bash
cd ~
git clone --recurse-submodules https://github.com/BottigerTheCynic/Think-OS-victor
cd Think-OS-victor
```

1. Source `setenv.sh`:

```bash
source setenv.sh
```

1. *(Optional)* Run this so you don’t have to source it every time:

```bash
echo "source \"$(pwd)/setenv.sh\"" >> $HOME/.bashrc
```

1. Build:

```bash
vbuild
```

</details>

<details>
<summary><strong>🍎 macOS (M-series only)</strong></summary>
<br />

**Prerequisites:** Make sure you have [Homebrew](https://brew.sh/) installed, then:

```bash
brew install ccache wget upx ninja pkg-config
```

1. Clone the repo and `cd` into it:

```bash
cd ~
git clone --recurse-submodules https://github.com/BottigerTheCynic/Think-OS-victor
cd Think-OS-victor
```

1. Run the build script:

```bash
./build/build-v.sh
```

</details>

-----

## Deploying

1. Install ThinkOS on your robot.
1. Get your robot’s IP through CCIS:
   1. Place your robot on the charger
   1. Double-click the button
   1. Lift the lift up then down
   1. Write down the IP address
   1. Lift the lift up then down again to exit CCIS
1. Deploy using one of the following:

<details>
<summary><strong>🐳 Docker: x86_64 or arm64 Linux &nbsp;|&nbsp; 🍎 macOS M-series</strong></summary>
<br />

```bash
./build/deploy-v.sh
```

</details>

<details>
<summary><strong>🖥️ Bare Metal: x86_64 or arm64 Linux</strong></summary>
<br />

```bash
vdeploy
```

</details>

-----

## Cleaning

99% of the time, if you’re working on a behavior or something, you don’t need to clean any build directories. The CMakeLists are correctly set up to properly rebuild only the code that needs to be rebuilt upon a file change.

If you do want to clean anyway:

<details>
<summary><strong>🐳 Docker: x86_64 or arm64 Linux &nbsp;|&nbsp; 🍎 macOS M-series</strong></summary>
<br />

```bash
./build/clean.sh
```

</details>

<details>
<summary><strong>🖥️ Bare Metal: x86_64 or arm64 Linux</strong></summary>
<br />

```bash
vclean
```

</details>

-----

## VSCode Code Completion

After you build for the first time, two files will be generated and placed in the root of the source directory:

- `compile_commands.json`
- `.clangd`

If you install the [`clangd`](https://marketplace.visualstudio.com/items?itemName=llvm-vs-code-extensions.vscode-clangd) extension for VSCode and relaunch it after a build, it will index the entire codebase and give you:

- Speedy code completion
- Error underlining and explanations
- Function descriptions and signatures

-----

## Contributing

Pull requests are welcome! Please make sure any new behavior or feature is productivity-focused or broadly applicable. Open an issue first if you’re unsure whether something fits the ThinkOS vision.
