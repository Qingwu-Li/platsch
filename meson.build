project('platsch', 'c')

# Define dependencies conditionally based on the HAVE_CAIRO option
platsch_dep = [dependency('libdrm', required: true)]
sources = ['platsch.c']
args = []

have_cairo = get_option('HAVE_CAIRO')

if have_cairo
    platsch_dep += dependency('cairo', required: true)
    sources += 'cairo.c'
    args += ['-DHAVE_CAIRO']
endif

# Define the headers
headers = ['platsch.h']

# Create the platsch executable
executable('platsch',
    sources,
    dependencies: platsch_dep,
    c_args: args,
    install: true,
    include_directories: include_directories('.')
)
