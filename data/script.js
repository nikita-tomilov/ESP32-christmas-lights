const colorSliderStep = 5;
const colorSliderMin = 0;
const colorSliderMax = 255;
const numberOfColors = 5;

const sadSliderStep = 100;
const sadSliderMin = 500;
const sadSliderMax = 5000;

const fadSliderStep = 5;
const fadSliderMin = 0;
const fadSliderMax = 100;

const igSliderStep = 0.1;
const igSliderMin = 0;
const igSliderMax = 2;

const cutoffSliderStep = 100;
const cutoffSliderMin = 150;
const cutoffSliderMax = 3050;

const setUp = (setupData) => {
    setupHeader(setupData);

    setUpBrightnessControls(setupData);
    setUpColorControls(setupData);

    /// sad controls setup: slider
    sliderSetup('slowAnimationDelayWrapper', sadSliderMin, sadSliderMax, sadSliderStep, setupData['slowAnimationDelay'], handleBrightnessSliderUpdate);
    
    /// fad controls setup: slider
    sliderSetup('fastAnimationDelayWrapper', fadSliderMin, fadSliderMax, fadSliderStep, setupData['fastAnimationDelay'], handleBrightnessSliderUpdate);
    
    if(setupData['fft'] == "true") {
        /// inputGain controls setup: slider
        sliderSetup('inputGainWrapper', igSliderMin, igSliderMax, igSliderStep, setupData['inputGain'], handleBrightnessSliderUpdate);
        
        /// cutoff controls setup: slider
        sliderSetup('cutoffWrapper', cutoffSliderMin, cutoffSliderMax, cutoffSliderStep, setupData['cutoff'], handleBrightnessSliderUpdate);
    } else {
        document.getElementById('inputGainWrapper').parentElement.style.display = 'none';
        document.getElementById('cutoffWrapper').parentElement.style.display = 'none';
    }

    const btn = document.getElementById('submit');
    btn.addEventListener('click', () => {
        const keys = Object.keys(setupData);
        const data = {};

        keys.forEach((k) => {
            const element = document.getElementById(k);
            if(element != null) {
                data[k] = element.value.substring(k.match(/color[0-9]/) ? 1 : 0);
            }
        });
        
        sendColorMode(marshall(data));
    });
}

const marshall = (data) => {
    const stringified = Object.entries(data).reduce((pr, cr, ind) => pr + '&' + cr[0]+'='+cr[1], '');
    return stringified.substring(1);
}

const setupHeader = (setupData) => {
    const header = document.getElementById('header');
    header.innerText = setupData['hw'] + ':' + setupData['ip'];

    const wrapper = document.getElementById('navigate');
    Object.entries(setupData['autodiscovery']).forEach((el) => {
        const link = document.createElement('a');
        link.href = 'http://' + el[1];
        link.innerText = el[0];

        wrapper.append(link);
    });
}

const setUpColorControls = (setupData) => {
    /// color controls setup: select
    const colorModes = setupData['colorModes'];
    const color = document.getElementById('colorMode');
    Object.entries(colorModes).forEach((el) => color.appendChild(getNewOptionItem(el[0], el[1])));
    color.value = setupData['colorMode'];

    const colorPicker = document.getElementById('colorPicker');

    for(var i = 0; i<numberOfColors; i++) {
        colorPicker.appendChild(getNewColorInput(setupData['color'+i], i));
    }
}

const getNewColorInput = (color, id) => {
    const newEl = document.createElement('input');
    newEl.className = 'colorPickerOption';
    newEl.type = 'color';
    newEl.id = 'color'+id;
    newEl.value = '#' + color;

    return newEl;
}

const setUpBrightnessControls = (setupData) => {
    const dataIds = {};

    /// brigthness controls setup: select
    const brightnessModes = setupData['brightnessModes'];
    const brightness = document.getElementById('brightnessMode');
    Object.entries(brightnessModes).forEach((el) => brightness.appendChild( getNewOptionItem(el[0], el[1], )));
    brightness.value = setupData['brightnessMode'];

    /// brigthness controls setup: slider
    sliderSetup('brightnessWrapper', colorSliderMin, colorSliderMax, colorSliderStep, setupData['brightness'], handleBrightnessSliderUpdate);
}

const sliderSetup = (wrapperId, min, max, step, value, handleSliderUpdate) => {
    const wrapper = document.getElementById(wrapperId);
    wrapper.className = 'sliderWrapper';

    /// slider setup
    const slider = document.createElement('input');
    slider.type = "range"; 
    slider.id = wrapperId.split('Wrapper')[0];
    slider.step = step;
    slider.min = min;
    slider.max = max;
    slider.value = value;
    slider.className = 'range';
    slider.setAttribute('list', wrapperId+'_ticks');

    /// slider ticks setup
    const sliderTicks = document.createElement('datalist');
    sliderTicks.id = wrapperId+'_ticks';
    for(var ind = 0; ind <= (max - min + 1)/step; ind++) {
        sliderTicks.appendChild(getNewOptionItem(min + ind * step, ''));
    }

    /// slider value text setup
    const text = document.createElement('div');
    text.className = 'sliderValue';
    const textId = wrapperId+'_value';
    text.id = textId;

    wrapper.appendChild(text);
    wrapper.appendChild(sliderTicks);
    wrapper.appendChild(slider);

    handleSliderUpdate(value, textId);
    slider.addEventListener('change', (ev) => handleSliderUpdate(ev.target.value, textId));
}

const handleBrightnessSliderUpdate = (newValue, valueElementId) => {
    const slider = document.getElementById(valueElementId);
    slider.innerText = newValue;
};

const getNewOptionItem = (value, title) => {
    const newEl = document.createElement('option');
    const text = document.createTextNode(title);
    newEl.value = value;

    newEl.appendChild(text);

    return newEl;
}

const sendColorMode = (params) => {
    updateLoader(false);
    document.getElementById('submit').disabled = true;

    const http = new XMLHttpRequest();
    const url = "/set/data";
    http.open("POST", url);
    http.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
    http.send(params);

    http.onreadystatechange = function() {
        updateLoader(true);
        document.getElementById('submit').disabled = false;
    }
}

const demarshall = (data) => {
    const tiles = data.replaceAll('\n', '').split('&');
    const dataObject = {};

    tiles.forEach((t) => {
        if(t[0] && t[1]){
            const parts = t.split('=');
            let value = parts[1];

            if(value.includes(":")){
                const innerDataObject = {};
                const innerParts = value.split(";");

                innerParts.forEach((ip) => {
                    const tmp = ip.split(':');
                    if(tmp[0] && tmp[1]) {
                        innerDataObject[tmp[0]] = tmp[1];
                    }
                });

                value = innerDataObject;
            }

            dataObject[parts[0]] = value;
        }
    });

    return dataObject;
} 

const updateLoader = (toHide) => {
    if(toHide) {
        document.getElementById('loader').style.display='none';
        document.getElementById('wrapper').style.display='flex';
    } else {
        document.getElementById('loader').style.display='block';
        document.getElementById('wrapper').style.display='none';
    }
} 

const getSetupData = () => {
    const http = new XMLHttpRequest();
    const url = "/get/data";
    http.onreadystatechange = function() {
        if (http.readyState === 4) {
            const response = http.response;
            document.getElementById('loader').style.display='none';
            document.getElementById('wrapper').style.display='flex';
            setUp(demarshall(response));
        }
      }
    http.open("GET", url);
    http.send();
}

getSetupData();



// const d = "fft=true&hw=ESP32-d84e6c&ip=192.168.0.164&autodiscovery=esp8266-2aa05f:192.168.0.127;esp8266-2bc220:192.168.0.168;&colorModes=0:SingleColor;1:TwoColors;2:FiveColors;3:Rainbow;4:Rainbow R;5:Rainbow CW;6:Rainbow CCW;7:FFT coarse;8:FFT fine;9:FFT coarse sym;10:FFT fine sym;&brightnessModes=0:SingleBrightness;1:RandomBrightnessBursts;2:FFT coarse;3:FFT fine;4:FFT coarse sym;5:FFT fine sym;&brightness=128&brightnessMode=0\n&colorMode=0&color0=FFFFFF&color1=FF0000&color2=00FF00&color3=FFFF00&color4=0000FF&slowAnimationDelay=1000&fastAnimationDelay=10&inputGain=1.00&cutoff=800";

// setTimeout(() => {
//     updateLoader(true);
//     setUp(demarshall(d));
// }, 2000)