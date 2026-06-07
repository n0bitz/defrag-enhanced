# defrag-enhanced
DeFRaG + Enhancements == defrag-enhanced

## Contributing

Requirements: `uv` and `git`

Initial setup:
```sh
git clone --recurse-submodules https://github.com/n0bitz/defrag-enhanced.git
cd defrag-enhanced
uv run pre-commit install
```

To build: `uv run build.py`


### Style

TODO: Put this somewhere else or get clang-* to enforce them automatically

Macro suffixes:
- "__" => private helper macros that can't be undef'd
- "_" => temp helper macros that'll be undef'd
- no suffix => public macros
