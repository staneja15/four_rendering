# Note: Run this scripts from the four_rendering base directory!

# compile basic shaders
slangc shaders/slang/basic.slang -o shaders/spirv/basic.spv

# compile grid shaders
slangc shaders/slang/grid.slang -o shaders/spirv/grid.spv
