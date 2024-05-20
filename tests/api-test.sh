addr="http://localhost:$(cat ./files/testdata/default.json | jq -r .port)"
auth="$(cat ./files/testdata/auth.json | jq -rc)"

# start phase
rm -f db.db3

$1/bin/backend --port "$(cat ./files/testdata/default.json | jq -r .port)" $2 &
sleep 1


# /register
curl -s -X 'POST' -d "$auth" $addr/user/register >> /dev/null

# /user/auth
token=$(curl -s -X 'POST' -d "$auth" $addr/user/auth)

# /user/ids/<n-m>
ids=$(curl -s -X 'GET' $addr/user/ids)
# get the one user id
uid=$(echo "$ids" | jq -r .[0])

# /user/data/update
userdata_json=$(cat ./files/testdata/userdata.json | jq -rc ".token = \"$token\"")
curl -s -X 'POST' -d "$userdata_json"  $addr/user/data/update >> /dev/null

# /user/get/<token>
userdata=$(curl -s -X 'GET' $addr/user/get/$token)

# /questions/defaults
questions=$(curl -s -X 'GET' $addr/questions/defaults)
qid=$(echo $questions | jq -r .[0].id)

# /questions/get/<tags>
default_questions=$(curl -s -X 'GET' $addr/questions/get/default)

# /questions/legends/<qid>
legends=$(curl -s -X 'GET' $addr/questions/legend/$qid)

# /journals/new
for i in $(seq 0 2); do
    curl -s -X 'POST' -d "$(cat ./files/testdata/journals.json | jq -rc "(.[$i].token = \"$token\")[$i]")" $addr/journals/new >> /dev/null
    sleep 1
done
# /journals/ids/<token>
jids=$(curl -s -X 'GET' $addr/journals/ids/$token)
# get the one journal
jid=$(echo "$jids" | jq -r .[0])

# /journals/get/<jid>
journal=$(curl -s -X 'GET' $addr/journals/get/$jid)

# /answers/get/<aid>/<token>
aid=$(echo "$journal" | jq -r .answers[0])
answer=$(curl -s -X 'GET' $addr/answers/get/$aid/$token)

# /settings/update
curl -s -X 'POST' -d "$(cat ./files/testdata/settings.json | jq -rc ".token = \"$token\"")" $addr/settings/update >> /dev/null

# /settings/get/<token>
settings=$(curl -s -X 'GET' $addr/settings/get/$token)

# /predictions/add
curl -s -X 'POST' -d "{\"token\":\"$token\"}" $addr/predictions/add >> /dev/null

# /predictions/get/<token>
prediction=$(curl -s -X 'GET' $addr/predictions/get/$token)

# /mitigations/tags/default
mitigations=$(curl -s -X 'GET' $addr/mitigations/tags/default)
mitigation=$(echo "$mitigations" | jq -r .[0].title)

# /mitigations/new/<token>
curated_mitigation=$(curl -s -X 'GET' $addr/mitigations/new/$token)

# /journals/delete/<jid>/<token>
#curl -s -X 'DELETE' $addr/journals/delete/$jid/$token

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
echo "auth:              $auth"
verify $auth
echo "token:             $token"
verify $token
echo "ids:               $ids"
verify $ids
echo "uid:               $uid"
verify $uid
echo "userdata_json:     $userdata_json"
verify $userdata_json
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
echo "legends:           $legends"
verify $legends
echo "prediction:        $prediction"
verify $prediction
echo "mitigations:       $mitigations"
verify $mitigations
echo "mitigation:        $mitigation"
verify $mitigation
echo "curated mitigation: $curated_mitigation"
verify $curated_mitigation

echo "all tests passed!"
exit 0
