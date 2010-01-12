from distutils.core import setup, Extension

setup( 
    name = "dynsql",
    description = "a sql template engine",
    author = "jayven",
    author_email = "kassarar@gmail.com",
    packages = ["dynsql",],
    ext_modules = [Extension("dynsql._dynsql", sources=["dynsql/_dynsql.c"]), ],
)
