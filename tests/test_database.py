import os
import sys
sys.path.append(f'{os.path.dirname(__file__)}/..')

import modules.database as database

# users table tests

def test_add_user():
    num_users = 1000
    for i in range(num_users):
        database.users.add(f'user{i}')
    
    assert len(database.users.all()) == num_users

def test_delete_user():
    database.users.add("user")
    user_id = database.users.all()[0]
    assert len(database.users.all()) == 1
    database.users.delete(user_id)
    assert len(database.users.all()) == 0

def test_modify_user():
    database.users.add("user")
    user_id = database.users.all()[0]
    current = database.users.getJSON(user_id)
    database.users.modify(user_id, name="user1")
    new = database.users.getJSON(user_id)
    assert not current == new

# journals table tests

def test_create_journal():
    database.users.add("user")
    user_id = database.users.all()[0]
    num_journals = 1000
    database.journals.add(user_id, [(f"q{i}", f"a{i}") for i in range(num_journals)])
    assert len(database.journals.all()) == num_journals
