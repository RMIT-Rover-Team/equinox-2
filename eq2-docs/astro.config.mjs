// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

import mdx from '@astrojs/mdx';

// https://astro.build/config
export default defineConfig({
  integrations: [
    starlight({
      title: "Equinox 2",
      social: [
        {
          icon: "github",
          label: "GitHub",
          href: "https://github.com/withastro/starlight",
        },
      ],
      sidebar: [
        {
          label: "Standards",
          items: [
            { label: "Git standards", slug: "standards/git"}
          ],
        },
        {
          label: "Guides",
          items: [
            // Each item here is one entry in the navigation menu.
            { label: "ROS2 Workshop", slug: "guides/ros2" },
          ],
        },
        {
          label: "Reference",
          autogenerate: { directory: "reference" },
        },
      ],
      customCss: ["./src/styles/custom.css"],
    }),
    mdx(),
  ],
});