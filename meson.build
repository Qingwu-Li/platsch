project('platsch', 'c')


# If SPINNER is true, force HAVE_CAIRO to be true
if get_option('SPINNER')
    message('SPINNER is enabled, forcing HAVE_CAIRO to be true')
    have_cairo = true
else
    have_cairo = get_option('HAVE_CAIRO')
endif

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

# Create the spinner executable if SPINNER true
if get_option('SPINNER')
    spinner_dep = [
        dependency('cairo', required: true),
        dependency('libdrm', required: true)
    ]

    spinner_src = [
        'spinner.c',
        'spinner_conf.c'
    ]
    executable('spinner',
        spinner_src,
        dependencies: spinner_dep,
        link_with: libplatsch,
        c_args: args,
        install: true,
        include_directories: include_directories('.')
    )
endif