import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/..')

import pytest

import modules.api as api

# authorization

@pytest.mark.skip(reason="Not implemented")
def test_register():
    API = api.API()
    username = "tester"
    password = "tester"
    result = API.register(username, password)
    assert result

@pytest.mark.skip(reason="Not implemented")
def test_login():
    API = api.API()
    username = "tester"
    password = "tester"
    API.register(username, password)
    token = API.login(username, password)
    assert not token == ""

@pytest.mark.skip(reason="Not implemented")
def test_get_initial_questions():
    API = api.API()
    username = "tester"
    password = "tester"
    API.register(username, password)
    token = API.login(username, password)
    assert API.get_initial(token) == [
        "How old are you?",
        "What is your occupation?",
        "Do you have children?" # ... fill in later
    ]


