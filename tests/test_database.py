import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/..')

import pytest
from json import loads

import modules.database as database

# load test data
with open("files/testdata/default.json", "r") as file:
    data = file.read()
    default_questions = loads(data)["questions"]
    users:dict[str, dict] = loads(data)["users"]

username = users.keys()[0]
password = users[username]["password"]

# users table tests

@pytest.mark.skip(reason="Not implemented")
def test_add_user():
    num_users = 1000
    for i in range(num_users):
        database.users.add(f'user{i}', passwd="test")
    
    assert len(database.users.all()) == num_users

@pytest.mark.skip(reason="Not implemented")
def test_delete_user():
    user_id = database.users.add(username, passwd=password)
    assert database.users.size() == 1
    database.users.delete(user_id)
    assert database.users.size() == 0

@pytest.mark.skip(reason="Not implemented")
def test_modify_user():
    user_id = database.users.add(username, passwd=password)
    current = database.users.getJSON(user_id)
    database.users.modify(user_id, name="user1")
    new = database.users.getJSON(user_id)
    assert not current == new

# journals table tests
@pytest.mark.skip(reason="Not implemented")
def test_create_journal():
    user_id = database.users.add(username, passwd=password)
    num_journals = 1000
    database.journals.add(user_id, [(f"q{i}", f"a{i}") for i in range(num_journals)])
    assert len(database.journals.all()) == num_journals
