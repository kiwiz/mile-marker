var Clay = require('pebble-clay')
var clayConfig = require('./config')
var clay = new Clay(clayConfig, null, {autoHandleEvents: false})
clay.registerComponent(require('./list-component'))

var markers = []

function doubleToByteArray(number) {
    var buffer = new ArrayBuffer(8)
    var longNum = new Float64Array(buffer)

    longNum[0] = number

    return Array.from(new Uint8Array(buffer))
}

function unpackMarkers(raw) {
    var buf = Uint8Array.from(raw).buffer

    var int_buf = new Uint16Array(buf, 0, 4)
    var size = int_buf[0]
    var chunk = int_buf[1]
    var count = int_buf[2]
    var more = int_buf[3] != 0

    var offset = int_buf.byteLength
    var markers = []
    for(var i = 0; i < count; ++i) {
        var slice = buf.slice(offset, offset + size)
        var marker_offset = 0
        var marker = {}

        var float_buf = new Float64Array(slice, marker_offset, 2)
        marker.latitude = float_buf[0]
        marker.longitude = float_buf[1]
        marker_offset += float_buf.byteLength

        var int_buf = new Uint32Array(slice, marker_offset, 1)
        marker.timestamp = int_buf[0]
        marker_offset += int_buf.byteLength

        var char_buf = new Uint8Array(slice, marker_offset, 10)
        var null_offset = char_buf.indexOf(0)
        if(null_offset > -1) {
            char_buf = new Uint8Array(slice, marker_offset, null_offset)
        }

        marker.emoji = new TextDecoder("utf-8").decode(char_buf)
        marker_offset += char_buf.byteLength

        markers.push(marker)
        offset += size
    }

    return {
        markers: markers,
        chunk: chunk,
        more: more
    }
}

Pebble.addEventListener('showConfiguration', function(e) {
    markers = []
    Pebble.sendAppMessage({'fetch': 0})
})

Pebble.addEventListener('ready', function() {
    Pebble.sendAppMessage({'ready': 0})
})

Pebble.addEventListener('appmessage', function(e) {
    if('pos' in e.payload) {
        var options = {
            enableHighAccuracy: true,
            maximumAge: 0,
            timeout: 3000,
        }

        navigator.geolocation.getCurrentPosition(function(pos) {
            Pebble.sendAppMessage({
                'lat': doubleToByteArray(pos.coords.latitude),
                'lon': doubleToByteArray(pos.coords.longitude),
            })
        }, null, options)
    } else if('data' in e.payload) {
        var chunk = unpackMarkers(e.payload['data'])
        markers = markers.concat(chunk.markers)

        if(chunk.more) {
            Pebble.sendAppMessage({'fetch': chunk.chunk + 1})
        } else {
            clay.meta.userData = markers
            Pebble.openURL(clay.generateUrl())
        }
    }
})
