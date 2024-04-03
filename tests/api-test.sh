addr="$(cat ./files/testdata/default.json | jq -r .addr)"
username="$(cat ./files/testdata/default.json | jq -r .user.username)"
password="$(cat ./files/testdata/default.json | jq -r .user.password)"
authjson="{\"username\":\"$username\", \"password\": \"$password\"}"

# /register
curl -X 'POST' -d "$authjson" $addr/register

# /auth
token=$(curl -X 'POST' -d "$authjson" $addr/auth)

# /user/ids/<n-m>
ids=$(curl -X 'GET' $addr/ids)
# get the one user id
uid=$(echo "$ids" | jq -r .[0])

# /user/data/update
curl -X 'POST' -d "{\"token\":\"$token\", \"data\":{}}" $addr/user/data/update

# /user/get/<uid>
userdata=$(curl -X 'GET' $addr/user/get/$uid)

# /journals/new
curl -X 'POST' -d "{\"token\":\"$token\", \"data\":[{\"question\":\"q1\", \"answer\":\"a1\"}]}" $addr/journals/new

# /journals/ids/<uid>
jids=$(curl -X 'GET' $addr/journals/ids/$uid)
# get the one journal
jid=$(echo "$jids" | jq -r .[0])

# /journals/get/<jid>
journal=$(curl -X 'GET' $addr/journals/get/$jid)

# /journals/delete/<uid>/<jid>
curl -X 'DELETE' $addr/journals/delete/$uid/$jid

# /settings/update
curl -X 'POST' -d "{\"token\":\"$token\", \"settings\":{\"key\":\"value\", \"key1\": \"value1\"}}" $addr/settings/update

# /settings/get/<uid>
settings=$(curl -X 'GET' $addr/settings/get/$uid)

# /questions/defaults
questions=$(curl -X 'GET' $addr/questions/defaults)

# /questions/get/<tags>
default_questions=$(curl -X 'GET' $addr/questions/get/default)

echo "---------------------TEST COMPLETE---------------------"
# verify if all data is intact
verify(){
    if [[ -z $1 ]]; then 
        echo "Error: variable is empty"
        exit 1
    fi
}
echo "addr:              $addr"
verify $addr
echo "username:          $username"
verify $username
echo "password:          $password"
verify $password
echo "authjson:          $authjson"
verify $authjson
echo "token:             $token"
verify $token
echo "ids:               $ids"
verify $ids
echo "uid:               $uid"
verify $uid
echo "userdata:          $userdata"
verify $userdata
echo "jids:              $jids"
verify $jids
echo "jid:               $jid"
verify $jid
echo "journal:           $journal"
verify $journal
echo "settings:          $settings"
verify $settings
echo "questions:         $questions"
verify $questions
echo "default questions: $default_questions"
verify $default_questions

echo "all tests passed!"
exit 0