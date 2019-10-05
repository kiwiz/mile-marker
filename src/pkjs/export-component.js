module.exports = {
    name: 'export',
    template: `
        <div class="component component-button">
            <button type="button">Copy to clipboard</button>
        </div>
    `,
    manipulator: {
        get: function() {},
        set: function() {},
    },
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

        function serialize(markers, json) {
            if(json) {
                return JSON.stringify(markers)
            }

            const data = [
                ['emoji', 'latitude', 'longitude', 'date']
            ]

            for(let i = 0; i < markers.length; ++i) {
                const date = (new Date(markers[i].timestamp * 1000))
                let latitude = Number.isNaN(markers[i].latitude) ? null:markers[i].latitude
                let longitude = Number.isNaN(markers[i].longitude) ? null:markers[i].longitude

                data.push([markers[i].emoji, latitude, longitude, dateStr(date)].join(','))
            }

            return data.join('\n')
        }

        this.$element.on('click', function(e) {
            e.preventDefault()

            const type_item = clayConfig.getItemById('export-type')
            const is_json = type_item.get() == 'json'

            const elem = document.createElement('textarea')
            elem.value = serialize(clayConfig.meta.userData, is_json)

            document.body.appendChild(elem)
            elem.select()
            document.execCommand("copy")
            document.body.removeChild(elem)
        })
    }
}
