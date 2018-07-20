
typedef struct material_props {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat shininess;
}material_props_t;

const int no_of_materials=24;

material_props_t materials[no_of_materials] = {
	{ //0 Emerald
		{ 0.0215f, 0.1745f, 0.0215f, 1.0f },//ambient;
		{ 0.07568f, 0.61424f, 0.07568f, 1.0f}, //diffuse
		{ 0.633f, 0.727811f, 0.633f, 1.0f},//specular
		 0.6f * 128//shininess
	},
	{ //1 Jade
		{0.135f, 0.2225f, 0.1575f, 1.0f},//ambient;
		{0.54f, 0.89f, 0.63f, 1.0f}, //diffuse
		{0.316228f, 0.316228f, 0.316228f, 1.0f},//specular
		 0.1f * 128//shininess
	},
	{ //2 Obsidian
		{ 0.05375f, 0.5f, 0.06625f, 1.0f },//ambient;
		{ 0.18275f, 0.17f, 0.22525f, 1.0f }, //diffuse
		{ 0.332741f, 0.328634f, 0.346435f, 1.0f },//specular
		0.3f * 128//shininess
	},
	{ //3 Pearl
		{ 0.25f, 0.20725f, 0.20725f, 1.0f },//ambient;
		{ 1.0f, 0.829f, 0.829f, 1.0f }, //diffuse
		{ 0.296648f, 0.296648f, 0.296648f, 1.0f },//specular
		0.88f * 128//shininess
	},
	{ //4 Ruby
		{ 0.1745f, 0.01175f, 0.01175f, 1.0f },//ambient;
		{ 0.61424f, 0.04136f, 0.04136f, 1.0f }, //diffuse
		{ 0.727811f, 0.626959f, 0.626959f, 1.0f },//specular
		0.6f * 128//shininess
	},
	{ //5 Turquoise
		{ 0.1f, 0.18725f, 0.1745f, 1.0f },//ambient;
		{ 0.396f, 0.74151f, 0.69102f, 1.0f }, //diffuse
		{ 0.297254f, 0.30829f, 0.306678f, 1.0f },//specular
		0.1f * 128//shininess
	},
	{ //6 Brass
		{ 0.329412f, 0.223529f, 0.27451f, 1.0f },//ambient;
		{ 0.78392f, 0.568627f, 0.113725f, 1.0f }, //diffuse
		{ 0.992157f, 0.941176f, 0.807843f, 1.0f },//specular
		0.21794872f * 128//shininess
	},
	{ //7 Bronze
		{ 0.2125f, 0.1275f, 0.054f, 1.0f },//ambient;
		{ 0.714f, 0.4284f, 0.18144f, 1.0f }, //diffuse
		{ 0.393548f, 0.271906f, 0.166721f, 1.0f },//specular
		0.2f * 128//shininess
	},
	{ //8 Chrome
		{ 0.25f, 0.25f, 0.25f, 1.0f },//ambient;
		{ 0.4f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.774597f, 0.774597f, 0.774597f, 1.0f },//specular
		0.6f * 128//shininess
	},
	{ //9 Copper
		{ 0.19125f, 0.0735f, 0.0225f, 1.0f },//ambient;
		{ 0.7038f, 0.27048f, 0.0828f, 1.0f }, //diffuse
		{ 0.25677f, 0.137622f, 0.086014f, 1.0f },//specular
		0.1f * 128//shininess
	},
	{ //10 Gold
		{ 0.24725f, 0.1995f, 0.0745f, 1.0f },//ambient;
		{ 0.75164f, 0.60648f, 0.22648f, 1.0f }, //diffuse
		{ 0.628281f, 0.555802f, 0.366065f, 1.0f },//specular
		0.4 * 128//shininess
	},
	{ //11 Silver
		{ 0.19225f, 0.19225f, 0.19225f, 1.0f },//ambient;
		{ 0.50745f, 0.50745f, 0.50745f, 1.0f }, //diffuse
		{ 0.508273f, 0.508273f, 0.508273f, 1.0f },//specular
		0.4f * 128//shininess
	},
	{ //12 Black
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.0f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.50f, 0.50f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //13 Cyan
		{ 0.0f, 0.1f, 0.06f, 1.0f },//ambient;
		{ 0.0f, 0.50980392f, 0.50980392f, 1.0f }, //diffuse
		{ 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },//specular
		0.25 * 128//shininess
	},
	{ //14 Green
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.1f, 0.35f, 0.1f, 1.0f }, //diffuse
		{ 0.45f, 0.55f, 0.45f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //15 Red
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.0f, 0.0f, 1.0f }, //diffuse
		{ 0.7f, 0.6f, 0.6f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //16 White
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.55f, 0.55f, 0.55f, 1.0f }, //diffuse
		{ 0.70f, 0.70f, 0.70f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //17 Yellow Plastic
		{ 0.0f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.0f, 1.0f }, //diffuse
		{ 0.60f, 0.60f, 0.50f, 1.0f },//specular
		0.25f * 128//shininess
	},
	{ //18 Black
		{ 0.02f, 0.02f, 0.02f, 1.0f },//ambient;
		{ 0.01f, 0.01f, 0.01f, 1.0f }, //diffuse
		{ 0.04f, 0.04f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //19 Cyan
		{ 0.0f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //20 Green
		{ 0.0f, 0.05f, 0.00f, 1.0f },//ambient;
		{ 0.4f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.04f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //21 Red
		{ 0.05f, 0.0f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.4f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.04f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //22 White
		{ 0.05f, 0.05f, 0.05f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.5f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.7f, 1.0f },//specular
		0.078125f * 128//shininess
	},
	{ //23 Yellow Rubber
		{ 0.05f, 0.05f, 0.0f, 1.0f },//ambient;
		{ 0.5f, 0.5f, 0.4f, 1.0f }, //diffuse
		{ 0.7f, 0.7f, 0.04f, 1.0f },//specular
		0.078125f * 128//shininess
	}
};
