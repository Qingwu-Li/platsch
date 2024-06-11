project('platsch', 'c')

have_cairo = get_option('HAVE_CAIRO')

# Define dependencies conditionally based on the HAVE_CAIRO option
platsch_dep = [dependency('libdrm', required: true)]
sources = ['libplatsch.c']
args = []

if have_cairo
    platsch_dep += dependency('cairo', required: true)
    sources += 'cairo.c'
    args += ['-DHAVE_CAIRO']
endif

# Create a static library from libplatsch.c and optionally cairo.c
libplatsch = static_library('libplatsch',
    sources,
    dependencies: platsch_dep,
    c_args: args,
    install: true
)

# Define the headers
headers = ['libplatsch.h', 'platsch.h']

# Create the platsch executable, linking it statically with libplatsch
executable('platsch',
    'platsch.c',
    dependencies: platsch_dep,
    c_args: args,
    link_with: libplatsch,
    install: true,
    include_directories: include_directories('.')
)
