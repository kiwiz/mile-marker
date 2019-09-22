Pebble.addEventListener('ready', function() {
    function doubleToByteArray(number) {
        var buffer = new ArrayBuffer(8);
        var longNum = new Float64Array(buffer);

        longNum[0] = number;

        return Array.from(new Uint8Array(buffer)).reverse();
    }

    var options = {
        enableHighAccuracy: true,
        maximumAge: 0,
        timeout: 3000,
    };

    Pebble.addEventListener('appmessage', function(e) {
        navigator.geolocation.getCurrentPosition(function(pos) {
            Pebble.sendAppMessage({
                'lat': doubleToByteArray(pos.coords.latitude),
                'lon': doubleToByteArray(pos.coords.longitude),
            });
        }, null, options);
    });

    Pebble.sendAppMessage({'ready': 0});
});
