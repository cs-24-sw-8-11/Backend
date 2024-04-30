addr="http://localhost:$(cat ./files/testdata/default.json | jq -r .port)"
username="$(cat ./files/testdata/default.json | jq -r .user.username)"
password="$(cat ./files/testdata/default.json | jq -r .user.password)"
authjson="{\"username\":\"$username\", \"password\": \"$password\"}"

# start phase
$1/bin/backend --port "$(cat ./files/testdata/default.json | jq -r .port)" &
sleep 1


# /register
curl -s -X 'POST' -d "$authjson" $addr/user/register >> /dev/null

# /user/auth
token=$(curl -s -X 'POST' -d "$authjson" $addr/user/auth)

# /user/ids/<n-m>
ids=$(curl -s -X 'GET' $addr/user/ids)
# get the one user id
uid=$(echo "$ids" | jq -r .[0])

# /user/data/update
curl -s -X 'POST' -d "{\"token\":\"$token\", \"data\":{\"agegroup\":\"42-69\", \"major\":\"school\"}}" $addr/user/data/update >> /dev/null

# /user/get/<token>
userdata=$(curl -s -X 'GET' $addr/user/get/$token)

# /questions/defaults
questions=$(curl -s -X 'GET' $addr/questions/defaults)
qid=$(echo $questions | jq -r .[0].id)

# /questions/get/<tags>
default_questions=$(curl -s -X 'GET' $addr/questions/get/default)

# /journals/new
curl -s -X 'POST' -d "{\"token\":\"$token\", \"comment\":\"Too many exams today.\", \"data\":[{\"question\":\"$qid\", \"answer\":\"8\"},{\"question\":\"2\", \"answer\":\"2\"}]}" $addr/journals/new >> /dev/null

# /journals/ids/<token>
jids=$(curl -s -X 'GET' $addr/journals/ids/$token)
# get the one journal
jid=$(echo "$jids" | jq -r .[0])

# /journals/get/<jid>
journal=$(curl -s -X 'GET' $addr/journals/get/$jid)

# /answers/get/<aid>/<token>
aid=$(echo "$journal" | jq -r .answers[0])
answer=$(curl -s -X 'GET' $addr/answers/get/$aid/$token)

# /journals/delete/<jid>/<token>
curl -s -X 'DELETE' $addr/journals/delete/$jid/$token

# /settings/update
curl -s -X 'POST' -d "{\"token\":\"$token\", \"settings\":{\"key\":\"modified\", \"key1\": \"modified1\",\"someKeyThatDoesntExist\": \"somevalue\"}}" $addr/settings/update >> /dev/null

# /settings/get/<token>
settings=$(curl -s -X 'GET' $addr/settings/get/$token)

# /predictions/add
curl -s -X 'POST' -d "{\"token\":\"$token\", \"questionid\":\"$qid\"}" $addr/predictions/add >> /dev/null

# /predictions/get/<uid>/<token>
prediction=$(curl -s -X 'GET' $addr/predictions/get/$token)

# /mitigations/tags/default
mitigations=$(curl -s -X 'GET' $addr/mitigations/tags/default)
mitigation=$(echo "$mitigations" | jq -r .[0])

pkill backend
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
echo "answer:            $answer"
verify $answer
echo "settings:          $settings"
verify $settings
echo "questions:         $questions"
verify $questions
echo "default questions: $default_questions"
verify $default_questions
echo "qid:               $qid"
verify $qid
echo "prediction:        $prediction"
verify $prediction
echo "mitigations:       $mitigations"
verify $mitigations
echo "mitigation:        $mitigation"
verify $mitigation

echo "all tests passed!"
exit 0
