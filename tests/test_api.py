import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/..')

import pytest
from json import loads

import modules.api as api

# load test data
with open("files/testdata/default.json", "r") as file:
    data = file.read()
    default_questions:list[str] = loads(data)["questions"]
    users:dict[str, dict] = loads(data)["users"]

username = list(users.keys())[0]
password = users[username]["password"]
API = api.API()

# authorization

@pytest.mark.skip(reason="Not implemented")
def test_register():
    result = API.register(username, password)
    assert result

@pytest.mark.skip(reason="Not implemented")
def test_login():
    API.register(username, password)
    token = API.login(username, password)
    assert not token == ""

@pytest.mark.skip(reason="Not implemented")
def test_get_initial_questions():
    API.register(username, password)
    token = API.login(username, password)
    assert API.get_initial(token) == default_questions


