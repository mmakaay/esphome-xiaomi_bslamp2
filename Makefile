help:
	@echo ""
	@echo "Targets:"
	@echo ""
	@echo "help:    Show this help"
	@echo "setup:   Setup tooling"
	@echo "check:   Run ruff check on code"
	@echo "format:  Perform ruff formatting on code"
	@echo ""

setup:
	@VIRTUAL_ENV="" uv run uv pip install -r pyproject.toml
	@VIRTUAL_ENV="" uv run pre-commit install

check:
	@VIRTUAL_ENV="" uv run ruff check

format:
	@VIRTUAL_ENV="" uv run ruff format

