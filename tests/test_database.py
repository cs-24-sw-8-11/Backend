import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/..')

import pytest
from json import loads

from modules.database.database import Database

# load test data
with open("files/testdata/default.json", "r") as file:
    data = file.read()
    default_questions = loads(data)["questions"]
    users:dict[str, dict] = loads(data)["users"]

username = list(users.keys())[0]
password = users[username]["password"]
if "db.db3" in os.listdir():
    os.remove("db.db3")
db = Database("db.db3")


# users table tests

def test_add_user():
    num_users = 1000
    for i in range(num_users):
        db.users.add(f'user{i}', "test")
    
    assert len(db.users) == num_users

@pytest.mark.skip(reason="Not implemented")
def test_delete_user():
    user_id = db.users.add(username, passwd=password)
    assert len(db.users) == 1
    db.users.delete(user_id)
    assert len(db.users) == 0

@pytest.mark.skip(reason="Not implemented")
def test_modify_user():
    user_id = db.users.add(username, passwd=password)
    current = db.users.getJSON(user_id)
    db.users.modify(user_id, name="user1")
    new = db.users.getJSON(user_id)
    assert not current == new

# journals table tests
def test_create_journal():
    user_id = db.users.add(username, password)
    num_journals = 1000
    for _ in range(num_journals):
        db.journals.add(user_id)
    assert len(db.journals) == num_journals
