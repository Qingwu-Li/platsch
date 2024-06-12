project('platsch', 'c')

platsch_dep = dependency('libdrm', version : '>= 2.4.112', required : true)
sources = ['platsch.c']

# Define the headers
headers = ['platsch.h']

# Create the platsch executable
executable('platsch',
    sources,
    dependencies: platsch_dep,
    install: true,
    include_directories: include_directories('.')
)
