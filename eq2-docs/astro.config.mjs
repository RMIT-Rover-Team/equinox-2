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
          items: [
            { label: "Git standards", slug: "standards/git" },
            { label: "CANBus standards", slug: "standards/canbus" }
          ],
        },
        {
          label: "Cheatsheets",
          items: [
            { label: "Git cheatsheet", slug: "guides/git-commands" },
            { label: "Linux cheatsheet", slug: "guides/linux-commands" }
          ],
        },
        {
          label: "Guides",
          items: [
            // Each item here is one entry in the navigation menu.
            { label: "Buzzword Basics", slug: "guides/buzzword-basics" },
            { label: "Git Basics Workshop", slug: "guides/git-basics" },
            { label: "ROS2 Workshop", slug: "guides/ros2" },
            { label: "CAN Bus Guide", slug: "guides/canbus-basics" },
            { label: "Packing and Unpacking", slug: "guides/pack-and-unpack" },
          ],
        },
        {
          label: "Reference",
          items: [
            { autogenerate: { directory: "reference" } }
          ],
        },
        {
          label: 'Design',
          items: [
            { label: "Excavator Hardware Interface", slug: "design/excavator/hardware-interface" },
            { label: "Science Hardware Interface", slug: "design/science/science-hardware-interface" },
          ],
        },
      ],
      customCss: ["./src/styles/custom.css"],
    }),
    mdx(),
  ],
});