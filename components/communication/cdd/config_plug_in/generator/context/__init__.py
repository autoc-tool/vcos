"""jinja2 context"""
import inspect
from .filters import Filter
from .functions import Function


def get_filters():
    """return all static functions from the Filters class."""

    return inspect.getmembers(Filter, predicate=inspect.isfunction)


def get_functions():
    """return all static functions in Functions class."""

    return inspect.getmembers(Function, predicate=inspect.isfunction)
