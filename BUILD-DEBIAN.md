# Building a .deb (Debian/Ubuntu)

## Build dependencies
```bash
sudo apt update
sudo apt install --no-install-recommends \
  build-essential debhelper-compat cmake pkg-config \
  libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```

## Build the package
From the project root:

```bash
dpkg-buildpackage -us -uc
```

This will create the .deb in the parent directory.

## Install
```bash
sudo dpkg -i ../infinitetux_*_*.deb
```

## Notes for maintainers
- `debian/control` and `debian/changelog` still contain placeholder maintainer fields.
  Replace them before publishing.
- For Debian main submission, the licensing information in `debian/copyright`
  must be made accurate and complete (DEP-5 format).
