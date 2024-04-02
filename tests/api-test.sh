addr="http://localhost:8080"
username="$(cat ./files/testdata/default.json | jq -r .user.username)"
password="$(cat ./files/testdata/default.json | jq -r .user.password)"
authjson="{"username":"$username", "password": "$password"}"

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
echo "addr:              $addr"
echo "username:          $username"
echo "password:          $password"
echo "authjson:          $authjson"
echo "token:             $token"
echo "ids:               $ids"
echo "uid:               $uid"
echo "userdata:          $userdata"
echo "jids:              $jids"
echo "jid:               $jid"
echo "journal:           $journal"
echo "settings:          $settings"
echo "questions:         $questions"
echo "default questions: $default_questions"

# verify if all data is intact
if [[ "$addr" == "" ]]; then
    exit 1
fi
if [[ "$username" == "" ]]; then
    exit 1
fi
if [[ "$password" == "" ]]; then
    exit 1
fi
if [[ "$authjson" == "" ]]; then
    exit 1
fi
if [[ "$token" == "" ]]; then
    exit 1
fi
if [[ "$ids" == "" ]]; then
    exit 1
fi
if [[ "$uid" == "" ]]; then
    exit 1
fi
if [[ "$userdata" == "" ]]; then
    exit 1
fi
if [[ "$jids" == "" ]]; then
    exit 1
fi
if [[ "$jid" == "" ]]; then
    exit 1
fi
if [[ "$journal" == "" ]]; then
    exit 1
fi
if [[ "$settings" == "" ]]; then
    exit 1
fi
if [[ "$questions" == "" ]]; then
    exit 1
fi
if [[ "$default_questions" == "" ]]; then
    exit 1
fi
exit 0