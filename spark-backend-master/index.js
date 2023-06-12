import express from 'express';
import * as http from 'http';
import WebSocket from 'ws';
import cors from 'cors'

const app = express();
app.use(express.json())
app.use(cors())

const server = http.createServer(app);
const wss = new WebSocket.Server({ server });


let parked = {}
let spots = [0, 1, 1, 0, 1, 0, 1, 1] // counter-clockwise
let socks = []
let crtSpot = 0;

let resetApp = ()=>{
    parked = {}
    spots = [0, 1, 1, 0, 1, 0, 1, 1] 
    crtSpot = 0;
}

let searchFree = ()=>{
    for(let i in spots)
    {
        if(spots[i] == 0)
            return i;
    }
    return -1;
}
var newCard = (sock, data) => {
    if(sock == "any")
    {
        for(sock of socks)
            if(sock.readyState === WebSocket.OPEN){
                newCard(sock, data)
                return;
            }
        console.log("no open socket");
        return;
    }
    if(parked[data.id] != null)
    {
        console.log(`retriving car in spot ${parked[data.id]}`);
        sock.send(`_spot ${parked[data.id]}`);
        crtSpot = parked[data.id];
        spots[parked[data.id]] = 0;
        parked[data.id] = null;
        console.log(parked);
    }
    else 
    {
        if(spots[data.currentSpot] == 0)
        {
            console.log(`new car saved in spot ${data.currentSpot}`)
            spots[data.currentSpot] = 1;
            parked[data.id] = data.currentSpot;
        }
        console.log("searching free spot...");
        let f = searchFree();
        if(f == -1)
        {
            console.log("no more space");
            sock.send('no more space');
        }
        else
        {
            console.log(`moving to new spot: ${f}`)
            sock.send(`_spot ${f}`);
            crtSpot = f;
        }
    }
    console.log("parked: ", parked);
    console.log("current spot: ", crtSpot);
}

wss.on('connection', (sock) => {
    console.log("new con");
    //connection is up, let's add a simple simple event
    socks.push(sock);
    sock.on('message', (message) => {

        //log the received message and send it back to the client
        sock.send(`server recived -> ${message}`);
        try{
            let data = JSON.parse(message);
            console.log(data);
            if(data.tag == 'newCard')
                newCard(sock, data);
        }
        catch{
            console.log("no Json data: " + message);
        }
    });
    //send immediatly a feedback to the incoming connection    
    sock.send('Hi there, I am a WebSocket server');

});

//start our server
server.listen(process.env.PORT || 3024, () => {
    console.log(`Server started on port ${server.address().port} :)`);
});



var stdin = process.openStdin();

stdin.addListener("data", function(d) {
    let x = (d.toString().trim())
    console.log("you entered: [" + 
        x + "]");
    if(x.split(' ')[0] == "register")
    {
        let data = {id: x[1], currentSpot: crtSpot}
        for(let sock of socks)
            newCard(sock, data)
    }
    else 
        for(let sock of socks)
            sock.send(x);
  });

app.post('/register', (req, res)=>{
    console.log("register requested from id: ", req.body);
    let data = req.body;
    data.currentSpot = crtSpot;
    // if(spots[crtSpot] != 0)
    //     res.send('no more space');
    newCard("any", data);
    res.send('registration request recived');
})
app.get('/reset', (req, res)=>{
    resetApp();
    console.log("resetted");
    res.send("done");
})
app.get('/status', (req, res)=>{
    console.log(spots);
    let availableSpots = 0;
    for(let i = 0; i < spots.length; i++){
        if(spots[i] == 0)
            availableSpots++;
    }
    res.send({'Available parking spots': " " + availableSpots});
})

// app.post('/retrive', (req, res)=>{
//     console.log("retrive requested from id: ", req.body);
//     let data = req.body;
//     data.currentSpot = crtSpot;
//     if(parked[data.id] == null)
//         res.send('not registered');
//     newCard("any", data);
//     res.send('retrieve request recived');
// })