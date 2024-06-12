project('platsch', 'c', version: '2024.06.0')

dep_libdrm = dependency('libdrm', version : '>= 2.4.112', required : true)
sources = ['platsch.c']

# Define the headers
headers = ['platsch.h']

# Create the platsch executable
executable('platsch',
    sources,
    dependencies: dep_libdrm,
    install: true,
    include_directories: include_directories('.')
)
