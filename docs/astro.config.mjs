// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

import mdx from '@astrojs/mdx';
import d2 from "astro-d2";

// https://astro.build/config
export default defineConfig({
  integrations: [starlight({
    title: "Equinox 2",
    favicon: "/rover-logo.png",
    logo: {
      light: "./public/equinox-logo-light.png",
      dark: "./public/equinox-logo-dark.png",
      replacesTitle: true,
    },
    social: [
      {
        icon: "github",
        label: "GitHub",
        href: "https://github.com/RMIT-Rover-Team/equinox-2",
      },
      {
        icon: "linkedin",
        label: "Linkedin",
        href: "https://au.linkedin.com/company/rmitroverteam",
      },
      {
        icon: "instagram",
        label: "Instagram",
        href: "https://www.instagram.com/rmitrover/"
      }
    ],
    sidebar: [
      {
        label: "Getting Started",
        items: [
          {
            label: "Quick start",
            slug: "getting-started/quick-start",
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
        label: "Guides",
        items: [
          { label: "Glossary", slug: "guides/glossary" },
          { label: "CAN Bus Guide", slug: "guides/canbus-basics" },
          { label: "Packing and Unpacking", slug: "guides/pack-and-unpack" },
          {
            label: "Workshops",
            items: [
              { label: "Git Basics Workshop", slug: "guides/workshops/git-basics" },
              { label: "ROS2 Workshop", slug: "guides/workshops/ros2" },
            ],
          },
          {
            label: "Cheatsheet",
            items: [
              { label: "Git Cheatsheet", slug: "guides/cheatsheets/git-commands" },
              { label: "Linux Cheatsheet", slug: "guides/cheatsheets/linux-commands" },
            ],
          },
        ],
      },
      {
        label: "Architecture",
        items: [
          {
            label: "Design",
            items: [],
          }
        ],
      },
    ],
    customCss: ["./src/styles/custom.css"],
    components: {
      // Head: "./src/components/head.astro",
      Header: "./src/components/header.astro",
      Sidebar: "./src/components/sidebar.astro",
    },
  }), mdx(), d2()],
});
