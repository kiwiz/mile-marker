
module.exports = {
    name: 'list',
    template: '<div class="section"></div>',
    manipulator: {
        get: function() {},
        set: function() {},
    },
    style:`
    .marker-component {display: grid; grid-template-columns: auto 1fr; grid-template-rows: auto auto}
    .marker-component .emoji {grid-row-start: span 2; font-size: 3em}
    .marker-component .location {align-self: center; text-align: right}
    .marker-component .date {align-self: center; text-align: right}
    `,
    initialize: function(minified, clayConfig) {
        function dateStr(date) {
            return [
                date.getFullYear(), '-',
                date.getMonth().toString().padStart(2, '0'), '-',
                date.getDate().toString().padStart(2, '0'),
                ' ',
                date.getHours().toString().padStart(2, '0'), ':' +
                date.getMinutes().toString().padStart(2, '0')
            ].join('')
        }

        const chunks = []
        const markers = clayConfig.meta.userData

        for(let i = 0; i < markers.length; ++i) {
            const date = (new Date(markers[i].timestamp * 1000))
            let location = 'Unavailable'
            if(!Number.isNaN(markers[i].latitude)) {
                location = markers[i].latitude, ', ', markers[i].longitude
            }

            chunks.push([
                '<div class="component marker-component">',
		    '<p class="emoji">', markers[i].emoji, '</p>',
		    '<p class="location">📍 ', location, '</p>',
		    '<p class="date">🕒 ', dateStr(date), '</p>',
                '</div>',
            ].join(''))
        }

        this.$element.ht(chunks.join(''))
    }
}
