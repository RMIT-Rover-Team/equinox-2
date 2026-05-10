// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

import mdx from '@astrojs/mdx';

// https://astro.build/config
export default defineConfig({
  integrations: [
    starlight({
      title: "Equinox 2",
      logo: {
        light: "./public/equinox-logo-light.png",
        dark: "./public/equinox-logo-dark.png",
        replacesTitle: true,
      },
      social: [
        {
          icon: "github",
          label: "GitHub",
          href: "https://github.com/withastro/starlight",
        },
      ],
      sidebar: [
        {
          label: "Start Here",
          items: [
            {
              label: "Getting started",
              slug: "start-here/getting-started",
            },
          ],
        },
        {
          label: "Standards",
          items: [{ label: "Git standards", slug: "standards/git" }],
        },
        {
          label: "Workshops",
          items: [
            // Each item here is one entry in the navigation menu.
            { label: "Git Basics Workshop", slug: "workshops/git-basics" },
            { label: "ROS2 Workshop", slug: "workshops/ros2" },
            { label: "CAN Bus Workshop", slug: "workshops/canbus-basics"}
          ],
        },
        {
          label: "Reference",
          items: [
            { autogenerate: { directory: "reference" } }
          ],
        },
      ],
      customCss: ["./src/styles/custom.css"],
    }),
    mdx(),
  ],
});