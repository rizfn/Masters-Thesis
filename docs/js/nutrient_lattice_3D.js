import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';


const { default: data } = await import('../data/nutrient/lattice3D_L=50_rho=1_delta=0.json', { assert: { type: "json" } });
// const { default: data } = await import('../data/nutrient/lattice3D_rho=1_delta=0.json', { assert: { type: "json" } });  // L=75




var step_list = data.reduce(function (a, d) {
    if (a.indexOf(d.step) === -1) {
        a.push(d.step);
    }
    return a;
}, []);

// on spacebar, call refilterdata
document.addEventListener('keydown', function(event) {
	if (event.code === 'Space') {
		refilter_data()
	}
});


data.forEach((d) => {
    const lattice = d.soil_lattice;
    const L = lattice.length
    // calculate the fraction of 1s, 2s, 3s and 0s in the 3D matrix
    const zeros = lattice.reduce((a, b) => a + b.reduce((c, d) => c + d.filter((x) => x === 0).length, 0), 0);
    const ones = lattice.reduce((a, b) => a + b.reduce((c, d) => c + d.filter((x) => x === 1).length, 0), 0);
    const twos = lattice.reduce((a, b) => a + b.reduce((c, d) => c + d.filter((x) => x === 2).length, 0), 0);
    const threes = lattice.reduce((a, b) => a + b.reduce((c, d) => c + d.filter((x) => x === 3).length, 0), 0);
    d.vacancy = zeros / L ** 3;
    d.nutrient = ones / L ** 3;
    d.soil = twos / L ** 3;
    d.worm = threes / L ** 3;
});

console.log(data);

let step = step_list[0]

let filteredData = data.filter(function (d) { return d.step == step });

// set the dimensions and margins of the graph
var margin = { top: 10, right: 10, bottom: 40, left: 40 },
    width = innerWidth / 2 - margin.left - margin.right,
    height = innerHeight - margin.top - margin.bottom;

// append the svg object to the body of the page
var svg = d3.select("div#raster")
    .append("svg")
    .attr("id", "soil_amounts")
    .attr("width", "100%")
    .attr("height", "100%")
    .append("g")
    .attr("transform",
        "translate(" + margin.left + "," + margin.top + ")");

// label axes
svg.append("text")
	.attr("class", "axis_label")
	.attr("transform", "translate(" + (width/2) + " ," + (height + margin.bottom/2) + ")")
	.style("text-anchor", "middle")
	.text("Death rate (θ)");

svg.append("text")
	.attr("class", "axis_label")
	.attr("transform", "rotate(-90)")
	.attr("y", 0 - margin.left/1.5)
	.attr("x",0 - (height / 2))
	.attr("dy", "1em")
	.style("text-anchor", "middle")
	.text("Soil-filling rate (σ)");


var rows = d3.map(data, function (d) { return d.theta; })
var cols = d3.map(data, function (d) { return d.sigma; })

var x = d3.scaleBand()
    .range([0, width])
    .domain(rows)
    .padding(0.05);

var y = d3.scaleBand()
    .range([height, 0])
    .domain(cols)
    .padding(0.05);

    function getOffset(element) {
        const rect = element.getBoundingClientRect();
        return {
          left: rect.left + window.scrollX,
          top: rect.top + window.scrollY
        };
      }
      

var Tooltip = d3.select("div#raster")
    .append("div")
    .style("opacity", 0)
    .attr("class", "tooltip")

// Three function that change the tooltip when user hover / move / leave a cell
var mouseover = function (event, d) {
    Tooltip
        .style("opacity", 1)
    d3.select(this)
        .style("stroke", "black")
}
var mousemove = function (event, d) {
    var heatmap_location = getOffset(document.getElementById("soil_amounts"))
    Tooltip
    .html("d=" + d3.format("0.2f")(d.theta) + ", " + "s=" + d3.format("0.2f")(d.sigma) + "<br>" + d3.format("0.2")(d.soil) + ", " + d3.format("0.2")(d.vacancy) + ", " + d3.format("0.2")(d.nutrient) + ", " + d3.format("0.2")(d.worm))
    .style("left", (d3.pointer(event)[0] + heatmap_location.left + 30) + "px")
        .style("top", (d3.pointer(event)[1] + heatmap_location.top - 20) + "px")
}
var mouseleave = function (event, d) {
    Tooltip
        .style("opacity", 0)
    d3.select(this)
        .style("stroke", "none")
}

let current_soil_lattice_state = { "theta": 0, "sigma": 0 };

// create a heatmap on mouseclick
var mousedown = function (event, d) {
    current_soil_lattice_state = { "theta": d.theta, "sigma": d.sigma };
    console.log(current_soil_lattice_state);
    update_soil_lattice(d.soil_lattice)
}

svg.selectAll(".cell")
    .data(filteredData)
    .enter()
    .append("rect")
    .attr("class", "cell")
    .attr("x", function (d) { return x(d.theta) })
    .attr("y", function (d) { return y(d.sigma) })
    .attr("width", x.bandwidth())
    .attr("height", y.bandwidth())
    .style("fill", function(d) { return "rgb(" + d.soil*255 + "," + d.vacancy*255 + "," + d.worm*255 + ")" } )
    .on("mouseover", mouseover)
    .on("mousemove", mousemove)
    .on("mouseleave", mouseleave)
    .on("mousedown", mousedown);



// var filteredData = data.filter(function (d) {
//     return d.sigma === 0.1578947368 && d.theta === 0.0947368421;
// });


let lattice = filteredData[0].soil_lattice;
const L = lattice.length;
console.log(lattice);

const canvas = d3.select("#c");

// Create a scene
var scene = new THREE.Scene();

// Create a camera
var camera = new THREE.PerspectiveCamera(75, window.innerWidth / window.innerHeight, 0.1, 1000);
camera.position.z = 1.5;

// Create a renderer
var renderer = new THREE.WebGLRenderer({ canvas: canvas.node() });
// renderer.setSize(window.innerWidth, window.innerHeight);
d3.select('div#lattice').node().appendChild(renderer.domElement);


// Create a directional light
var light = new THREE.DirectionalLight(0xffffff, 2);
camera.add(light);

// Add the camera to the scene
scene.add(camera);


// Create a group
var group = new THREE.Group();


// Select the div to hold the checkboxes
var controlsDiv = d3.select('div#controls');

// Define the checkbox data
var checkboxes = ['empty', 'nutrient', 'soil', 'worm'];

// Bind the data to the divs
var divSelection = controlsDiv.selectAll('div')
    .data(checkboxes)
    .enter()
    .append('div');

// Append a checkbox to each div
const checkboxSelection = divSelection.append('input')
    .attr('type', 'checkbox')
    .attr('id', function (d) { return d; });

// Append a label to each div
divSelection.append('label')
    .attr('for', function (d) { return d; })
    .text(function (d) { return d; });


// Listen for change events on the checkboxes
checkboxSelection.on('change', function (event, d) {
    // Remove all meshes from the group
    while (group.children.length > 0) {
        group.remove(group.children[0]);
    }

    // Add a mesh for each checked checkbox
    checkboxSelection.each(function (d) {
        var checkbox = d3.select(this);
        if (checkbox.property('checked')) {
            var target_site;
            var color;

            switch (d) {
                case 'empty':
                    target_site = 0;
                    color = 0xffffff; // white
                    break;
                case 'nutrient':
                    target_site = 1;
                    color = 0xadd8e6; // light blue
                    break;
                case 'soil':
                    target_site = 2;
                    color = 0xa54a2a; // brown
                    break;
                case 'worm':
                    target_site = 3;
                    color = 0x008000; // green
                    break;
            }

            var mesh = createMesh(target_site, color);
            group.add(mesh);
        }
    });
});

d3.select("#soil").property("checked", true).dispatch("change");


// Create a function to create a mesh for a given target site and color
function createMesh(target_site, color) {
    // Create a geometry
    var geometry = new THREE.BoxGeometry(1, 1, 1);
    // Create a material
    var material = new THREE.MeshStandardMaterial({ color: color, transparent: true, opacity: 1 });

    // Calculate the number of target sites
    var targetCount = 0;
    for (let i = 0; i < L; i++) {
        for (let j = 0; j < L; j++) {
            for (let k = 0; k < L; k++) {
                if (lattice[i][j][k] === target_site) {
                    targetCount++;
                }
            }
        }
    }

    // Create an InstancedMesh with the correct number of instances
    var mesh = new THREE.InstancedMesh(geometry, material, targetCount);

    // Loop over the lattice and position each instance
    var instance = 0;
    for (let i = 0; i < L; i++) {
        for (let j = 0; j < L; j++) {
            for (let k = 0; k < L; k++) {
                if (lattice[i][j][k] === target_site) {
                    var matrix = new THREE.Matrix4().makeScale(1 / L, 1 / L, 1 / L).premultiply(new THREE.Matrix4().makeTranslation(i / L - 0.5, j / L - 0.5, k / L - 0.5));
                    mesh.setMatrixAt(instance, matrix);
                    instance++;
                }
            }
        }
    }

    return mesh;
}


// Add the group to the scene
scene.add(group);

// Create controls
var controls = new OrbitControls(camera, renderer.domElement);
controls.enableZoom = true;
controls.enableRotate = true;
controls.enablePan = true;


function update_soil_lattice(newData) {
    // Remove all meshes from the group
    while (group.children.length > 0) {
        group.remove(group.children[0]);
    }

    // Update the lattice data
    lattice = newData;

    // Add a mesh for each checked checkbox
    checkboxSelection.each(function(d) {
        var checkbox = d3.select(this);
        if (checkbox.property('checked')) {
            var target_site;
            var color;

            switch (d) {
                case 'empty':
                    target_site = 0;
                    color = 0xffffff; // white
                    break;
                case 'nutrient':
                    target_site = 1;
                    color = 0xadd8e6; // light blue
                    break;
                case 'soil':
                    target_site = 2;
                    color = 0xa52a2a; // brown
                    break;
                case 'worm':
                    target_site = 3;
                    color = 0x008000; // green
                    break;
            }

            var mesh = createMesh(target_site, color);
            group.add(mesh);
        }
    });
}



function resizeRendererToDisplaySize(renderer) {
    const canvas = renderer.domElement;
    const pixelRatio = window.devicePixelRatio;
    const width = canvas.clientWidth * pixelRatio | 0;
    const height = canvas.clientHeight * pixelRatio | 0;
    const needResize = canvas.width !== width || canvas.height !== height;
    if (needResize) {
        renderer.setSize(width, height, false);
    }
    return needResize;
}



// Create a function to animate the scene
function animate() {
    requestAnimationFrame(animate);
    // group.rotation.x += 0.001;
    // group.rotation.y += 0.001;
    controls.update();
    if (resizeRendererToDisplaySize(renderer)) {
        const canvas = renderer.domElement;
        camera.aspect = canvas.clientWidth / canvas.clientHeight;
        camera.updateProjectionMatrix();
    }
    renderer.render(scene, camera);
}

// Start the animation
animate();


function refilter_data() {
	
	const t = d3.transition().duration(750)

	console.log('Refiltering data')

	// select the next entry in steplist
	step = step_list[(step_list.indexOf(step) + 1) % step_list.length]

	filteredData = data.filter(function(d){ return d.step == step });

	// add a 0.5s popup window  to show the current step
	var popup = d3.select("div#visualization")
		.append("div")
		.attr("class", "popup")	
		.html(step)
		.transition(t)
		.style("opacity", 1)
		.transition(t)
		.style("opacity", 0)
		.remove();

	// update the rgb heatmap
	svg.selectAll(".cell")
		.data(filteredData)
		.transition(t)
		.style("fill", function(d) { return "rgb(" + d.soil*255 + "," + d.vacancy*255 + "," + d.worm*255 + ")" } )

	const soil_lattice = filteredData.filter(function(d) {return d.theta == current_soil_lattice_state.theta && d.sigma == current_soil_lattice_state.sigma})[0].soil_lattice
	// update the lattice
	update_soil_lattice(soil_lattice)
}
