from check_syntax import pyflakes_wrapper

__all__ = [pyflakes_wrapper]

__all__.sort(key=lambda module: module.priority, reverse=True)