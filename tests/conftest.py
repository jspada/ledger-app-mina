import pytest

def pytest_addoption(parser):
    parser.addoption(
        "--all", action="store_true", default=False, help="Run all tests including crypto tests (not for release builds)"
    )

def pytest_configure(config):
    config.addinivalue_line("markers", "all: only run for not release builds")

def pytest_collection_modifyitems(config, items):
    if config.getoption("--all"):
        return
    skip = pytest.mark.skip(reason="Use --all flag to run")
    for item in items:
        if "all" in item.keywords:
            item.add_marker(skip)
